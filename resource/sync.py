import sqlite3

def float_equal(x, y, tolerance=1e-2):
    return abs(x - y) < tolerance

musicInfos = {}

LEVEL_MAPPING = {
    0: "rating_ez",
    1: "rating_hd",
    2: "rating_in",
    3: "rating_at",
    4: "rating_lg"
}

if __name__ == "__main__":
    conn = sqlite3.connect("./PhigrosInfo.db")
    
    levels = {}

    # 打开文件
    with open('difficulty.tsv', 'r', encoding='utf-8') as difficulty:
        # 逐行读取文件内容
        for single in difficulty:
            level = single.strip().split('\t')
            level[0] = f"{level[0]}.0"
            for index in range(1,len(level)):
                level[index] = float(level[index])
            if len(level) < 5:
                level.append(None)
            levels[level[0]] = level[1:]
    
    # 打开文件
    with open('info.tsv', 'r', encoding='utf-8') as infos:
        # 逐行读取文件内容
        for single in infos:
            musicInfo = single.strip().split('\t', 8)
            if len(musicInfo) < 8:
                musicInfo.append(None)
                
            info = {
                "sid": musicInfo[0],
                "title": musicInfo[1],
                "artist": musicInfo[2],
                "illustration": musicInfo[3],
                "charts": [musicInfo[4], musicInfo[5], musicInfo[6], musicInfo[7]]
            }
            musicInfos[f"{musicInfo[0]}.0"] = info
    
    for key, value in levels.items():
        info = conn.execute(f'SELECT * FROM phigros WHERE sid = "{key}"').fetchall()
        info = info[0]
        compare_level = (info[5], value[0]),(info[6], value[1]),(info[7], value[2]),(info[8], value[3])

        # 效验level
        for index in range(len(compare_level)):
            val = compare_level[index]
            if None is val[0]:
                continue
            if not float_equal(val[0], val[1]):
                change_sql = f"""UPDATE phigros SET {LEVEL_MAPPING[index]} = {val[1]} where sid = "{key}";"""
                print(change_sql)
                print(f"{key} : {val[0]} => {val[1]}")
                conn.execute(change_sql)
                conn.commit()
                
        # 效验info
        musicInfo = musicInfos[key]
        # 17谱师
        sid, title, artist, illustration = info[0], info[2], info[23], info[24]
        charts = [info[17], info[18], info[19], info[20]]
        conditionCharts = charts == musicInfo["charts"] 
        conditionTitle = title == musicInfo['title']
        conditionArtist = artist == musicInfo['artist']
        conditionIllustration = illustration == musicInfo['illustration']
        
        if not conditionCharts:
            print(f'[{musicInfo["sid"]}] 谱师错误')
            print(f'{charts}\n{musicInfo["charts"]}\n')
            
        if not conditionTitle:
            print(f'[{musicInfo["sid"]}] 标题错误')
            print(f'{title}\n{musicInfo["title"]}\n')
            
        if not conditionArtist:
            print(f'[{musicInfo["sid"]}] 曲师错误')
            print(f'{artist}\n{musicInfo["artist"]}\n')
            
        if not conditionIllustration:
            print(f'[{musicInfo["sid"]}] 画师错误')
            print(f'{illustration}\n{musicInfo["illustration"]}\n')
            