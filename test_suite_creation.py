import os
import random
import time
import shutil
import string
import argparse



NOW = int(time.time())
DAY = 86400

# Tunable counts
NUM_SIGNALS_PER_CATEGORY = {
    "FINANCE": 200,   # you asked for high-signal finance
    "HR": 120,
    "LEGAL": 40,
    "IT": 40,
    "TRAPS": 20
}

NOISE_COUNTS = {
    "SYSTEM_LIB": 400,
    "USER_DOCS": 700,
    "DEV_JUNK": 1500
}

# realistic user list (sampled from your dataset)
USERS = [
    "ybrown","hubbardkimberly","scottjonathan","nrodriguez","sara02","emily49","phillipbolton",
    "michael53","patricia94","jbarnes","kmorris","fmiller","arthur13","xclark","marygonzalez",
    "lelaurie","kristenmartin","vanessaburton","awilliams","goodjeffrey","root","andrea41",
    "david97","jacobjones","awilliams","wmartinez","zphillips","nholmes","awilliams","elaine"
]

# per-extension typical size ranges in KB (min, max)
EXT_SIZE_KB = {
    ".xlsx": (10, 800),    # spreadsheets often 10KB - 800KB (can be larger)
    ".xlsm": (50, 6000),   # macro-enabled often bigger
    ".xls": (10, 400),
    ".csv": (5, 3000),
    ".docx": (5, 1500),
    ".pdf": (5, 5000),
    ".pptx": (50, 5000),
    ".txt": (1, 50),
    ".zip": (50, 5000),
    ".rtf": (5, 200)
}

# helper templates for finance file names (realistic)
FINANCE_TEMPLATES = [
    "FY{y}_{dept}_Budget_v{v}.xlsx",
    "{dept}_Forecast_{month}{y}.xlsx",
    "AP_Aging_{month}{y}.csv",
    "AR_Collections_{m}_{y}.xlsx",
    "GL_Export_{date}.csv",
    "Cash_Position_{date}.xlsx",
    "Tax_Workpaper_{entity}_{y}.xlsx",
    "CapEx_Request_{proj}_{v}.xlsx",
    "Vendor_Payment_Run_{date}.xlsx",
    "Forecast_Model_FINAL_v{v}.xlsm",
    "Close_Package_{period}.pdf",
    "Expense_Accruals_{month}{y}.csv",
    "Bank_Recon_{bank}_{m}{y}.xlsx"
]

HR_TEMPLATES = [
    "CV_{lastname}_{y}.docx",
    "{firstname}_Resume_{y}.docx",
    "Employee_Benefits_{y}.xlsx",
    "Onboarding_Checklist_{team}.xlsx",
    "Salary_Scales_{y}_FINAL.xlsx",
    "Severance_Agreement_{name}.docx",
    "Interview_Notes_{candidate}.docx"
]

LEGAL_TEMPLATES = [
    "{plaintiff} v. {defendant} No. {num}-CV-{id}.docx",
    "Order Granting Motion to Amend Complaint - {case}.docx",
    "{case}_findings_and_conclusions.docx",
    "Class Notice of Compromise - {case}.docx",
    "Opinion_{court}_{date}.docx"
]

IT_TEMPLATES = [
    "prod_db_dump_{date}.sql.gz",
    "wildcard_cert_{domain}.key",
    ".env.production",
    "keepass_backup_{date}.kdbx",
    "settings_{service}.json"
]

# realistic departments / months / banks / projects used in templates
DEPARTMENTS = ["Ops","Sales","Finance","HR","R&D","Regional","Corp"]
MONTHS = ["Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"]
BANKS = ["Chase","BofA","WellsFargo","Citi"]
PROJECTS = ["ProjectA","Apollo","Mercury","CANY","RD"]
ENTITIES = ["Corp","Subsidiary","Region1","Region2"]

# legal name pairs and case numbers
PLAINTIFFS = ["Smith","Jones","AcmeCorp","Miller","Johnson"]
DEFENDANTS = ["Doe","GlobalBank","XYZInc","Bank of America","Lehigh"]
CASE_NUMS = [f"{random.randint(1,99)}-{random.randint(1000,9999)}" for _ in range(200)]

def rand_user():
    return random.choice(USERS)

def rand_days_ago(min_days=1, max_days=1000):
    return random.randint(min_days, max_days)

def pick_extension_for_template(template):
    # preference by keywords
    if "csv" in template or "CSV" in template:
        return ".csv"
    if "pdf" in template:
        return ".pdf"
    if "xlsm" in template:
        return ".xlsm"
    if "Forecast_Model" in template or "Model" in template:
        return random.choice([".xlsm", ".xlsx"])
    # otherwise prefer spreadsheet for finance and HR
    return random.choice([".xlsx", ".xls", ".csv"])

def size_for_ext(ext):
    if ext not in EXT_SIZE_KB:
        return random.randint(5*1024, 200*1024)
    lo, hi = EXT_SIZE_KB[ext]
    return random.randint(lo*1024, hi*1024)

def mkpath(*parts):
    # produce folder strings using Windows-like style inside base dir
    return os.path.join(*parts)

def safe_mkdir_for(fullpath):
    os.makedirs(os.path.dirname(fullpath), exist_ok=True)

def write_file(full_path, ext, size_bytes):
    safe_mkdir_for(full_path)
    with open(full_path, "wb") as f:
        # write small realistic header where possible
        if ext == ".pdf":
            f.write(b"%PDF-1.4\n")
            f.write(os.urandom(min(size_bytes, 512)))
        elif ext in (".docx", ".xlsx", ".pptx", ".xlsm", ".zip"):
            f.write(b"PK\x03\x04")       # minimal ZIP container header
            f.write(os.urandom(min(size_bytes, 512)))
        elif ext == ".csv" or ext == ".txt" or ext == ".rtf":
            # write readable-ish text
            sample = b"col1,col2,col3\n" + os.urandom(64)
            f.write(sample[:min(len(sample), size_bytes)])
            if size_bytes > len(sample):
                # pad with random ascii digits/commas/newlines
                pad = ''.join(random.choices(string.ascii_letters + ",\n 0123456789", k=min(size_bytes-len(sample), 512))).encode('utf-8')
                f.write(pad)
        else:
            f.write(os.urandom(min(size_bytes, 256)))
        # ensure file reaches target size
        current = f.tell()
        if size_bytes > current:
            f.truncate(size_bytes)

def set_mtime(full_path, days_ago):
    mtime = NOW - days_ago * DAY
    os.utime(full_path, (mtime, mtime))

# realistic path builders (these produce relative folder components that emulate C:\Users\... inside BASE_DIR)
def finance_path_for(user):
    # various realistic finance-ish paths, including OneDrive
    choices = [
        f"Users/{user}/Documents/Finance/Reporting",
        f"Users/{user}/OneDrive - Corp/Finance/Forecasts",
        f"Users/{user}/Downloads",
        f"Shared/Finance/Uploads",
        f"Finance/Temp",
        f"Users/{user}/Desktop",
        f"Users/{user}/Documents/Finance/Tax"
    ]
    return random.choice(choices)

def hr_path_for(user):
    choices = [
        f"Users/{user}/Documents/HR/Payroll",
        f"Users/{user}/OneDrive - Corp/HR/Handbooks",
        f"HR/Recruiting/Candidates",
        f"Users/{user}/Downloads",
        f"HR/Policies",
        f"Users/{user}/Desktop"
    ]
    return random.choice(choices)

def legal_path_for(user):
    choices = [
        f"Users/{user}/Documents/Legal",
        f"Shared/Legal/Cases",
        f"Users/{user}/Downloads",
        f"Legal/Archive",
        f"Users/{user}/Desktop"
    ]
    return random.choice(choices)

def it_path_for(user):
    choices = [
        f"Infra/Backups",
        f"Users/{user}/AppData/Roaming",
        f"Users/{user}/Documents/Keys",
        f"Infra/Certificates",
        f"Users/{user}/Desktop"
    ]
    return random.choice(choices)

# name generators
def gen_finance_filename():
    tpl = random.choice(FINANCE_TEMPLATES)
    y = random.choice([2023,2024,2025])
    v = random.randint(1,7)
    month = random.choice(MONTHS)
    date = time.strftime("%Y%m%d", time.localtime(NOW - random.randint(0, 400)*DAY))
    dept = random.choice(DEPARTMENTS)
    proj = random.choice(PROJECTS)
    entity = random.choice(ENTITIES)
    bank = random.choice(BANKS)
    return tpl.format(y=y, v=v, month=month, date=date, dept=dept, proj=proj, entity=entity, bank=bank, period=f"{month}{y}", m=month, projcode=proj)

def gen_hr_filename():
    tpl = random.choice(HR_TEMPLATES)
    y = random.choice([2023,2024,2025])
    firstname = random.choice(["John","David","Maria","Sara","Emily","Michael","Laura","Jason","Anna","Victor"])
    lastname = random.choice(["Smith","Johnson","Brown","Rodriguez","Nguyen","Patel","Lopez","Gonzalez"])
    team = random.choice(["Engineering","Sales","Support","Product"])
    candidate = f"{firstname}_{lastname}"
    return tpl.format(y=y, firstname=firstname, lastname=lastname, team=team, candidate=candidate, name=f"{firstname}_{lastname}")

def gen_legal_filename():
    tpl = random.choice(LEGAL_TEMPLATES)
    plaintiff = random.choice(PLAINTIFFS)
    defendant = random.choice(DEFENDANTS)
    case = f"{plaintiff}_{defendant}_{random.randint(2000,2025)}"
    num = random.choice(CASE_NUMS)
    court = random.choice(["DC","NY","CA","FL"])
    date = time.strftime("%Y%m%d", time.localtime(NOW - random.randint(0, 1300)*DAY))
    return tpl.format(plaintiff=plaintiff, defendant=defendant, num=num, id=random.randint(100,9999), case=case, court=court, date=date)

def gen_it_filename():
    tpl = random.choice(IT_TEMPLATES)
    date = time.strftime("%Y%m%d", time.localtime(NOW - random.randint(0, 800)*DAY))
    domain = random.choice(["corp.local","example.com","prod.service"])
    service = random.choice(["auth","db","cache","api"])
    return tpl.format(date=date, domain=domain, service=service)

# main create helpers for categories
def create_signals_for_finance(count):
    for i in range(count):
        user = rand_user()
        folder = finance_path_for(user)
        name = gen_finance_filename()
        # pick extension sensible to the template
        ext = os.path.splitext(name)[1]
        if ext == "":
            ext = random.choice([".xlsx", ".xlsm", ".csv", ".pdf", ".docx"])
            name = name + ext
        # sometimes add copy suffix or "FINAL"
        if random.random() < 0.15:
            suffix = random.choice([" FINAL"," - FINAL"," copy"," (1)"," v2"])
            name = name.replace(ext, "") + suffix + ext
        size = size_for_ext(ext)
        days_old = rand_days_ago(1, 1200)
        full_path = os.path.join(BASE_DIR, folder, name)
        write_file(full_path, ext, size)
        set_mtime(full_path, days_old)
        if i < 5 and random.random() < 0.6:
            # also create a CSV export variant in a messy Downloads folder to mimic user behavior
            alt_name = name.replace(ext, "") + "_export.csv"
            alt_path = os.path.join(BASE_DIR, f"Users/{user}/Downloads", alt_name)
            write_file(alt_path, ".csv", size_for_ext(".csv"))
            set_mtime(alt_path, rand_days_ago(1,1200))

def create_signals_for_hr(count):
    for i in range(count):
        user = rand_user()
        folder = hr_path_for(user)
        name = gen_hr_filename()
        ext = os.path.splitext(name)[1] or random.choice([".docx",".xlsx",".pdf"])
        if not name.endswith(ext):
            name = name + ext
        # add CV ambiguity sometimes: inject "-CV-" style or "CV" with hyphen-number to create labeling confusion
        if random.random() < 0.12:
            name = name.replace(ext, "") + f" -CV-{random.randint(1,999)}" + ext
        # sometimes legal-looking "severance" or "compromise" sits in HR, produce such borderline cases
        if random.random() < 0.08:
            name = f"Severance_Order_{random.randint(100,999)}{ext}"
        size = size_for_ext(ext)
        full_path = os.path.join(BASE_DIR, folder, name)
        write_file(full_path, ext, size)
        set_mtime(full_path, rand_days_ago(1, 1200))

def create_signals_for_legal(count):
    for i in range(count):
        user = rand_user()
        folder = legal_path_for(user)
        name = gen_legal_filename()
        # ensure these have .docx or .pdf usually
        ext = random.choice([".docx",".pdf"])
        if not name.endswith(ext):
            name = name.replace(".docx", "")  # strip if template already had .docx
            name = name + ext
        # sometimes include "ORDER" or "MTD" tokens in upper/lower
        if random.random() < 0.25:
            name = name.replace(ext, "") + f" - ORDER_{random.randint(1,999)}" + ext
        size = size_for_ext(ext)
        full_path = os.path.join(BASE_DIR, folder, name)
        write_file(full_path, ext, size)
        set_mtime(full_path, rand_days_ago(1, 2500))

def create_signals_for_it(count):
    for i in range(count):
        user = rand_user()
        folder = it_path_for(user)
        name = gen_it_filename()
        ext = os.path.splitext(name)[1] or random.choice([".key", ".kdbx", ".sql.gz", ".json"])
        if not name.endswith(ext):
            name = name + ext
        size = size_for_ext(ext if ext in EXT_SIZE_KB else ".txt")
        full_path = os.path.join(BASE_DIR, folder, name)
        write_file(full_path, ext, size)
        set_mtime(full_path, rand_days_ago(1, 1200))

def create_traps():
    traps = [
        ("Windows/Temp/Diagnostic_Results", "system_resource_finance.csv"),
        ("Users/Dev/Project/node_modules/debug/src", "settings.json"),
        ("Windows/WinSxS/budget_component", "core_budget_api.dll"),
        ("Program Files/Common Files/Adobe/Cache", "invoice_template.dat"),
        ("Users/Public/Pictures", "id_rsa_backup_preview.jpg"),
        ("Users/User_1/AppData/Local/Temp", "f8a2c1.key"),
    ]
    for folder, name in traps:
        full = os.path.join(BASE_DIR, folder, name)
        ext = os.path.splitext(name)[1] or ".dat"
        write_file(full, ext, size_for_ext(ext if ext in EXT_SIZE_KB else ".txt"))
        set_mtime(full, rand_days_ago(1, 900))

def create_noise():
    # system libs
    for _ in range(NOISE_COUNTS["SYSTEM_LIB"]):
        path = os.path.join(BASE_DIR, f"Windows/System32/{get_random_string(6)}", f"{get_random_string(8)}.dll")
        write_file(path, ".dll", size_for_ext(".zip"))  # use .zip size mapping for larger binaries
        set_mtime(path, rand_days_ago(30, 4000))

    # user docs (diverse ext)
    user_exts = [".pdf", ".docx", ".txt", ".jpg", ".png", ".zip", ".mp3", ".xlsx"]
    for _ in range(NOISE_COUNTS["USER_DOCS"]):
        u = rand_user()
        folder = random.choice([
            f"Users/{u}/Documents/Old", f"Users/{u}/Downloads", f"Users/{u}/Desktop", f"Users/{u}/OneDrive - Corp/Misc",
            f"Users/{u}/Documents/misc/{random.choice(['old','final','new folder','backup'])}"
        ])
        fname = f"document_{get_random_string(5)}{random.choice(user_exts)}"
        full = os.path.join(BASE_DIR, folder, fname)
        ext = os.path.splitext(fname)[1]
        write_file(full, ext, size_for_ext(ext if ext in EXT_SIZE_KB else ".txt"))
        set_mtime(full, rand_days_ago(1, 4000))

    # dev junk
    for _ in range(NOISE_COUNTS["DEV_JUNK"]):
        path = os.path.join(BASE_DIR, f"Users/Dev/Workspace/node_modules/lib_{random.randint(1,400)}", f"{get_random_string(6)}.js")
        write_file(path, ".js", random.randint(1*1024, 50*1024))
        set_mtime(path, rand_days_ago(1, 1200))

# small helper used by create_noise
def get_random_string(length=8):
    return ''.join(random.choices(string.ascii_lowercase + string.digits, k=length))

# MAIN: build lab
def create_lab():
    print(f"Rebuilding lab at: {BASE_DIR}")
    if os.path.exists(BASE_DIR):
        shutil.rmtree(BASE_DIR)
    os.makedirs(BASE_DIR, exist_ok=True)

    # 1) Signals: finance-heavy, hr, legal, it
    print("Creating finance signals...")
    create_signals_for_finance(NUM_SIGNALS_PER_CATEGORY["FINANCE"])
    print("Creating HR signals...")
    create_signals_for_hr(NUM_SIGNALS_PER_CATEGORY["HR"])
    print("Creating LEGAL confusers...")
    create_signals_for_legal(NUM_SIGNALS_PER_CATEGORY["LEGAL"])
    print("Creating IT signals...")
    create_signals_for_it(NUM_SIGNALS_PER_CATEGORY["IT"])

    # 2) traps
    print("Creating contextual traps...")
    create_traps()

    # 3) background noise
    print("Creating background noise (system, user docs, dev junk)...")
    create_noise()

    print("Lab creation complete.")

if __name__ == "__main__":
    # command-line argument parsing for test directory path
    parser = argparse.ArgumentParser(description="CyberML Test Lab Generator")
    parser.add_argument("--path", type=str, default="./test_data", help="Path to generate the Test Directory")
    args = parser.parse_args()

    BASE_DIR = args.path

    print(f"Rebuilding Test Directory at: {BASE_DIR}...")

    if not os.path.exists(BASE_DIR):
        os.makedirs(BASE_DIR)

    create_lab()
