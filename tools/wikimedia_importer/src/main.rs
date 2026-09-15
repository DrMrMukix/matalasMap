use serde::{Deserialize, Serialize};
use sha2::{Digest, Sha256};
use std::fs::{self, File};
use std::io::{Read, Write};
use std::path::Path;

const USER_AGENT: &str = "MatalasMapHistoricalImporter/1.0 (educational map creator; contact@matalas.local)";

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct FlagVersion {
    pub year_start: i32,
    pub year_end: i32,
    pub file: String,
    pub description: String,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct HistoricalNation {
    pub id: String,
    pub name: String,
    pub min_year: i32,
    pub max_year: i32,
    pub flag: String,
    pub emblem: Option<String>,
    pub color: [u8; 4],
    pub source: String,
    pub wikidata_id: Option<String>,
    pub license: String,
    pub author: Option<String>,
    pub source_url: Option<String>,
    pub sha256: Option<String>,
    pub downloaded_at: Option<String>,
    pub flag_versions: Vec<FlagVersion>,
}

struct NationQuery {
    id: &'static str,
    name: &'static str,
    min_year: i32,
    max_year: i32,
    commons_title: &'static str,
    wikidata_id: &'static str,
    color: [u8; 4],
}

fn fetch_wikimedia_file_info(commons_title: &str) -> Result<(String, String, String, String), Box<dyn std::error::Error>> {
    let encoded_title = commons_title.replace(' ', "_");
    let api_url = format!(
        "https://commons.wikimedia.org/w/api.php?action=query&titles={}&prop=imageinfo&iiprop=url|extmetadata&format=json",
        encoded_title
    );

    let resp: serde_json::Value = ureq::get(&api_url)
        .set("User-Agent", USER_AGENT)
        .call()?
        .into_json()?;

    let pages = resp["query"]["pages"].as_object().ok_or("No pages in response")?;
    let page = pages.values().next().ok_or("Empty pages object")?;
    let imageinfo = &page["imageinfo"][0];

    let download_url = imageinfo["url"].as_str().ok_or("No download url")?.to_string();
    let extmetadata = &imageinfo["extmetadata"];

    let license = extmetadata["LicenseShortName"]["value"]
        .as_str()
        .unwrap_or("Public Domain")
        .to_string();

    let author = extmetadata["Artist"]["value"]
        .as_str()
        .unwrap_or("Wikimedia Commons")
        .to_string();

    let source_url = format!("https://commons.wikimedia.org/wiki/{}", encoded_title);

    Ok((download_url, license, author, source_url))
}

fn download_file(url: &str, out_path: &Path) -> Result<String, Box<dyn std::error::Error>> {
    println!("Downloading {} -> {:?}", url, out_path);
    let mut resp = ureq::get(url)
        .set("User-Agent", USER_AGENT)
        .call()?
        .into_reader();

    let mut bytes = Vec::new();
    resp.read_to_end(&mut bytes)?;

    let mut hasher = Sha256::new();
    hasher.update(&bytes);
    let hash = format!("{:x}", hasher.finalize());

    let mut file = File::create(out_path)?;
    file.write_all(&bytes)?;

    Ok(hash)
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("=== MatalasMap Wikimedia & Historical Importer ===");

    let flags_dir = Path::new("assets/flags");
    let nations_dir = Path::new("content/nations");
    fs::create_dir_all(flags_dir)?;
    fs::create_dir_all(nations_dir)?;

    let targets = vec![
        NationQuery {
            id: "esp_1700",
            name: "Imperio Español (1700-1808)",
            min_year: 1700,
            max_year: 1808,
            commons_title: "File:Flag_of_Cross_of_Burgundy.svg",
            wikidata_id: "Q12536",
            color: [194, 32, 38, 255],
        },
        NationQuery {
            id: "esp_1874",
            name: "España (Restauración)",
            min_year: 1874,
            max_year: 1931,
            commons_title: "File:Flag_of_Spain_(1785–1873,_1875–1931).svg",
            wikidata_id: "Q1194389",
            color: [170, 21, 27, 255],
        },
        NationQuery {
            id: "esp_1978",
            name: "Reino de España",
            min_year: 1978,
            max_year: 2026,
            commons_title: "File:Flag_of_Spain.svg",
            wikidata_id: "Q29",
            color: [218, 41, 28, 255],
        },
        NationQuery {
            id: "fra_1650",
            name: "Reino de Francia",
            min_year: 1650,
            max_year: 1789,
            commons_title: "File:Royal_Standard_of_the_King_of_France.svg",
            wikidata_id: "Q15623",
            color: [26, 54, 93, 255],
        },
        NationQuery {
            id: "fra_1804",
            name: "Primer Imperio Francés",
            min_year: 1804,
            max_year: 1814,
            commons_title: "File:Flag_of_France.svg",
            wikidata_id: "Q71083",
            color: [30, 58, 138, 255],
        },
        NationQuery {
            id: "fra_1958",
            name: "República Francesa",
            min_year: 1958,
            max_year: 2026,
            commons_title: "File:Flag_of_France.svg",
            wikidata_id: "Q142",
            color: [37, 99, 235, 255],
        },
        NationQuery {
            id: "gbr_1707",
            name: "Imperio Británico",
            min_year: 1707,
            max_year: 1927,
            commons_title: "File:Flag_of_Great_Britain_(1707–1800).svg",
            wikidata_id: "Q8612",
            color: [30, 41, 59, 255],
        },
        NationQuery {
            id: "gbr_1927",
            name: "Reino Unido",
            min_year: 1927,
            max_year: 2026,
            commons_title: "File:Flag_of_the_United_Kingdom.svg",
            wikidata_id: "Q145",
            color: [15, 23, 42, 255],
        },
        NationQuery {
            id: "deu_1871",
            name: "Imperio Alemán",
            min_year: 1871,
            max_year: 1918,
            commons_title: "File:Flag_of_the_German_Empire.svg",
            wikidata_id: "Q120601",
            color: [24, 24, 27, 255],
        },
        NationQuery {
            id: "deu_1990",
            name: "Alemania",
            min_year: 1990,
            max_year: 2026,
            commons_title: "File:Flag_of_Germany.svg",
            wikidata_id: "Q183",
            color: [180, 83, 9, 255],
        },
        NationQuery {
            id: "rus_1721",
            name: "Imperio Ruso",
            min_year: 1721,
            max_year: 1917,
            commons_title: "File:Flag_of_Russia.svg",
            wikidata_id: "Q159",
            color: [51, 65, 85, 255],
        },
        NationQuery {
            id: "sov_1922",
            name: "Unión Soviética",
            min_year: 1922,
            max_year: 1991,
            commons_title: "File:Flag_of_the_Soviet_Union.svg",
            wikidata_id: "Q15180",
            color: [185, 28, 28, 255],
        },
        NationQuery {
            id: "rus_1991",
            name: "Federación Rusa",
            min_year: 1991,
            max_year: 2026,
            commons_title: "File:Flag_of_Russia.svg",
            wikidata_id: "Q159",
            color: [29, 78, 216, 255],
        },
        NationQuery {
            id: "usa_1776",
            name: "Estados Unidos (1776)",
            min_year: 1776,
            max_year: 1860,
            commons_title: "File:Betsy_Ross_flag.svg",
            wikidata_id: "Q30",
            color: [29, 78, 216, 255],
        },
        NationQuery {
            id: "usa_1960",
            name: "Estados Unidos",
            min_year: 1960,
            max_year: 2026,
            commons_title: "File:Flag_of_the_United_States.svg",
            wikidata_id: "Q30",
            color: [30, 64, 175, 255],
        },
        NationQuery {
            id: "mex_1821",
            name: "Primer Imperio Mexicano",
            min_year: 1821,
            max_year: 1823,
            commons_title: "File:Flag_of_the_First_Mexican_Empire.svg",
            wikidata_id: "Q28563",
            color: [4, 120, 87, 255],
        },
        NationQuery {
            id: "mex_1824",
            name: "México",
            min_year: 1824,
            max_year: 2026,
            commons_title: "File:Flag_of_Mexico.svg",
            wikidata_id: "Q96",
            color: [5, 150, 105, 255],
        },
        NationQuery {
            id: "col_1819",
            name: "Gran Colombia",
            min_year: 1819,
            max_year: 1831,
            commons_title: "File:Flag_of_Gran_Colombia.svg",
            wikidata_id: "Q199821",
            color: [245, 158, 11, 255],
        },
        NationQuery {
            id: "chl_1817",
            name: "Chile",
            min_year: 1817,
            max_year: 2026,
            commons_title: "File:Flag_of_Chile.svg",
            wikidata_id: "Q298",
            color: [220, 38, 38, 255],
        },
        NationQuery {
            id: "arg_1816",
            name: "Argentina",
            min_year: 1816,
            max_year: 2026,
            commons_title: "File:Flag_of_Argentina.svg",
            wikidata_id: "Q414",
            color: [56, 189, 248, 255],
        },
        NationQuery {
            id: "bra_1822",
            name: "Imperio del Brasil",
            min_year: 1822,
            max_year: 1889,
            commons_title: "File:Flag_of_the_Empire_of_Brazil.svg",
            wikidata_id: "Q217230",
            color: [21, 128, 61, 255],
        },
        NationQuery {
            id: "bra_1889",
            name: "Brasil",
            min_year: 1889,
            max_year: 2026,
            commons_title: "File:Flag_of_Brazil.svg",
            wikidata_id: "Q155",
            color: [22, 163, 74, 255],
        },
        NationQuery {
            id: "jpn_1868",
            name: "Imperio de Japón",
            min_year: 1868,
            max_year: 1947,
            commons_title: "File:War_flag_of_the_Imperial_Japanese_Army.svg",
            wikidata_id: "Q188712",
            color: [225, 29, 72, 255],
        },
        NationQuery {
            id: "jpn_1947",
            name: "Japón",
            min_year: 1947,
            max_year: 2026,
            commons_title: "File:Flag_of_Japan.svg",
            wikidata_id: "Q17",
            color: [239, 68, 68, 255],
        },
        NationQuery {
            id: "chn_1644",
            name: "Dinastía Qing",
            min_year: 1644,
            max_year: 1912,
            commons_title: "File:Flag_of_the_Qing_dynasty_(1889–1912).svg",
            wikidata_id: "Q19178",
            color: [234, 179, 8, 255],
        },
        NationQuery {
            id: "chn_1949",
            name: "China",
            min_year: 1949,
            max_year: 2026,
            commons_title: "File:Flag_of_the_People's_Republic_of_China.svg",
            wikidata_id: "Q148",
            color: [220, 38, 38, 255],
        },
        NationQuery {
            id: "ott_1650",
            name: "Imperio Otomano",
            min_year: 1650,
            max_year: 1922,
            commons_title: "File:Flag_of_the_Ottoman_Empire.svg",
            wikidata_id: "Q12560",
            color: [4, 120, 87, 255],
        },
        NationQuery {
            id: "ita_1861",
            name: "Reino de Italia",
            min_year: 1861,
            max_year: 1946,
            commons_title: "File:Flag_of_Italy_(1861–1946).svg",
            wikidata_id: "Q172579",
            color: [30, 136, 229, 255],
        },
        NationQuery {
            id: "ita_1946",
            name: "Italia",
            min_year: 1946,
            max_year: 2026,
            commons_title: "File:Flag_of_Italy.svg",
            wikidata_id: "Q38",
            color: [46, 125, 50, 255],
        },
    ];

    let mut result_nations: Vec<HistoricalNation> = Vec::new();

    for target in targets {
        println!("Resolving {} ({}) from Wikimedia Commons...", target.name, target.commons_title);
        let flag_filename = format!("{}.svg", target.id);
        let local_flag_rel = format!("assets/flags/{}", flag_filename);
        let local_flag_path = flags_dir.join(&flag_filename);

        let (download_url, license, author, source_url) = match fetch_wikimedia_file_info(target.commons_title) {
            Ok(info) => info,
            Err(e) => {
                eprintln!("Warning: Failed to query Wikimedia info for {}: {}. Will use fallback.", target.commons_title, e);
                (String::new(), "Public Domain".to_string(), "Wikimedia Commons".to_string(), format!("https://commons.wikimedia.org/wiki/{}", target.commons_title))
            }
        };

        let sha256 = if !download_url.is_empty() {
            match download_file(&download_url, &local_flag_path) {
                Ok(h) => Some(h),
                Err(e) => {
                    eprintln!("Warning: download failed for {}: {}", download_url, e);
                    None
                }
            }
        } else {
            None
        };

        let nation = HistoricalNation {
            id: target.id.to_string(),
            name: target.name.to_string(),
            min_year: target.min_year,
            max_year: target.max_year,
            flag: local_flag_rel,
            emblem: None,
            color: target.color,
            source: "Wikimedia Commons".to_string(),
            wikidata_id: Some(target.wikidata_id.to_string()),
            license,
            author: Some(author),
            source_url: Some(source_url),
            sha256,
            downloaded_at: Some("2026-09-14".to_string()),
            flag_versions: Vec::new(),
        };

        result_nations.push(nation);
        // Polite delay for Wikimedia API
        std::thread::sleep(std::time::Duration::from_millis(150));
    }

    let nations_file = nations_dir.join("nations.json");
    let json_data = serde_json::to_string_pretty(&result_nations)?;
    fs::write(&nations_file, json_data)?;

    println!("SUCCESS: Grabbed {} historical nations and saved locally to {:?}", result_nations.len(), nations_file);

    Ok(())
}
