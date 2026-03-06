import numpy as np
import pandas as pd
from sklearn.feature_extraction.text import HashingVectorizer
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import accuracy_score, classification_report
from scipy.sparse import hstack
import os

if not os.path.exists("master_training_dataset.csv"):
    raise FileNotFoundError("master_training_dataset.csv not found")

df = pd.read_csv("master_training_dataset.csv")
output_path = "include/ModelWeights.h"

# Feature Engineering:
print("Performing feature engineering, this may take a moment...")

# Fixed Epoch: 01/01/2026
REF_DATE = 1767225600 
ONE_YEAR = 365 * 86400

df["recency_score"] = 1 / ((REF_DATE - df["mod_time_unix"]) / ONE_YEAR + 1)
df["size_logged"] = np.log1p(df["size_bytes"])
df["name_len"] = df["filename"].apply(len)
df["path_len"] = df["path"].apply(len)
df["path_depth"] = df["path"].apply(lambda p: p.count("/") + p.count("\\"))

# Initialize HashingVectorizer
hashing_vectorizer = HashingVectorizer(
    n_features=2048,
    analyzer="char",
    norm=None,
    alternate_sign=False,
    ngram_range=(4, 4),
)

# Apply feature hashing to the 'filename' column
filename_vectors = hashing_vectorizer.fit_transform(df["filename"])
path_vectors = hashing_vectorizer.fit_transform(df["path"])

# Training:
print("Training models for each target label...")

# Define target-specific valuable extensions
target_extension_map = {
    "label_finance": [".xlsx", ".xls", ".csv", ".pdf", ".accp", ".pptx"],
    "label_hr": [".docx", ".doc", ".rtf", ".pdf", ".xlsx", ".xls"],
    "label_it": [".pem",".key",".kdbx",".p12",".ovpn",".private",".wallet",".sql",".env"]
}

# Add "label_general" after defining the other labels
target_extension_map["label_general"] = list(
    set(
        target_extension_map["label_finance"]
        + target_extension_map["label_hr"]
        + target_extension_map["label_it"]
    )
)
common_dev_junk = [".pyc", ".pyi", ".lcl", ".pyd", ".map", ".ts", ".js", ".spec", ".test.js",]

# Define target-specific junk extensions
target_junk_map = {
    "label_finance": [".class", ".java", ".cpp", ".obj", ".dll", ".exe"]
    + common_dev_junk,
    "label_hr": [".class", ".java", ".py", ".obj", ".dll", ".exe"] + common_dev_junk,
    "label_it": [".tmp", ".log", ".cache"]
    + [ext for ext in common_dev_junk if ext not in [".ts", ".js"]],
}

target_junk_map["label_general"] = list(
    set(target_junk_map["label_finance"]
        + target_junk_map["label_hr"]
        + target_junk_map["label_it"])
)

target_configs = {
    "label_finance": {"C": 0.3, "weight": 4.0},
    "label_general": {"C": 0.3, "weight": 2.0},
    "label_hr":      {"C": 0.5, "weight": 4.0},
    "label_it":      {"C": 0.8, "weight": 12.0}
}

models_map = {}

for target, config in target_configs.items():

    # Prepare Data
    df_temp = df.copy()
    df_temp["valuable_ext"] = df_temp["extension"].isin(target_extension_map[target]).astype(int)
    df_temp["junk_ext"] = df_temp["extension"].isin(target_junk_map[target]).astype(int)

    # Create feature matrix X and target vector y for the current target
    numerical_features = df_temp[
        [
            "recency_score",
            "size_logged",
            "valuable_ext",
            "junk_ext",
            "name_len",
            "path_len",
            "path_depth",
        ]
    ].values
    X_target = hstack([numerical_features, filename_vectors, path_vectors])
    y_target = df_temp[target].values

    # Split Data
    X_train, X_test, y_train, y_test, idx_train, idx_test = train_test_split(
        X_target, y_target, df_temp.index, test_size=0.2, random_state=42, stratify=y_target)

    # Train Model
    C = config["C"]
    model = LogisticRegression(solver="lbfgs", max_iter=1000,
                                C=C, class_weight={0: 1, 1: config['weight']})
    model.fit(X_train, y_train)

    # Evaluate
    y_pred = model.predict(X_test)
    y_scores = model.predict_proba(X_test)[:, 1]

    print("Accuracy for", target, ":", accuracy_score(y_test, y_pred))
    print("\nClassification Report for c =", C, ", weight =", config['weight'], ", Bias =", model.intercept_[0], ":\n", classification_report(y_test, y_pred))

    models_map[target] = model


# Exporting Model Parameters to C++ Header:

print("Exporting model parameters to C++ header file...")

# Ensure Directory Exists and Write
os.makedirs(os.path.dirname(output_path), exist_ok=True)

cpp_content = """
#pragma once
// AUTOMATICALLY GENERATED FILE 

#include <vector>
#include <unordered_set>
#include <string>
"""

for target, model in models_map.items():

    # Extract Parameters
    bias = model.intercept_[0]
    weights = model.coef_.flatten()
    title = target.split("_")[1].upper()
    cpp_content += f"""
const double {title}_MODEL_BIAS = {bias:.10f};

// recency score - size_logged - valuable_ext -  junk_ext - name_len - path_len - path_depth - filename features(2048) - path features(2048)
const std::vector<double> {title}_MODEL_WEIGHTS = {{
{', '.join([f'{w:.10f}' for w in weights])}}};

const std::unordered_set<std::wstring> {title}_HIGH_VAL_EXTS = {{
{', '.join([f'L"{ext}"' for ext in target_extension_map[target]])}}};

const std::unordered_set<std::wstring> {title}_JUNK_EXTS = {{
{', '.join([f'L"{ext}"' for ext in target_junk_map[target]])}}};
"""

with open(output_path, "w") as f:
    f.write(cpp_content)

print(f"\nSUCCESS! Model weights exported to: {os.path.abspath(output_path)}")
print("You can now rebuild the C++ payload to apply the new weights.")
