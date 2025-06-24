import sqlite3,json

# 用于判断数据库是否有问题的脚本

def connect_sql():
    return sqlite3.connect("./PhigrosInfo.db")

def float_equal(x, y, tolerance=1e-2):
    return abs(x - y) < tolerance

level_list = ("EZ","HD","IN","AT","Legacy")

if __name__ == "__main__":
    # 打开并读取JSON文件  
    with open('Modified_Gameinfo.json', 'r', encoding='utf-8') as file:  
        # 使用json.load()函数从文件中加载JSON数据  
        data = json.load(file)  
    
    for sid, info in data["regularSongs"].items():
        print(sid)
        database_info = connect_sql().execute(f'SELECT * FROM phigros WHERE sid = "{sid}"').fetchall()
        database_info = database_info[0]
        # print(database_info)
        title, composer, illustrator = database_info[2], database_info[23], database_info[24]
        # ez, hd ,in ,at, lg
        ratings = (database_info[5], database_info[6], database_info[7], database_info[8], database_info[9])        
        notes = (database_info[11], database_info[12], database_info[13], database_info[14], database_info[15])
        charts = (database_info[17], database_info[18], database_info[19], database_info[20], database_info[21])
        
        EZ = HD = IN = AT = LG = {
            "rating": None,
            "charter": None,
            "numOfNotes": None,
        }
        
        chart_detail = info["chartDetail"]
        if chart_detail.get("EZ") is not None:
            EZ = chart_detail["EZ"]
        if chart_detail.get("HD") is not None:
            HD = chart_detail["HD"]
        if chart_detail.get("IN") is not None:
            IN = chart_detail["IN"]
        if chart_detail.get("AT") is not None:
            AT = chart_detail["AT"]
        if chart_detail.get("Legacy") is not None:
            LG = chart_detail["Legacy"]
        
        
        
        info_ratings = (EZ["rating"], HD["rating"], IN["rating"], AT["rating"], LG["rating"])
        info_notes = (EZ["numOfNotes"], HD["numOfNotes"], IN["numOfNotes"], AT["numOfNotes"], LG["numOfNotes"])
        info_charts = (EZ["charter"], HD["charter"], IN["charter"], AT["charter"], LG["charter"])
        
        conditionTitle = title == info['songsName']
        conditionArtist = composer == info['composer']
        conditionIllustration = illustrator == info['illustrator']
        
        if not conditionTitle:
            print(f'[{sid}] 标题错误')
            print(f'{title}\n{info["songsName"]}\n')
            
        if not conditionArtist:
            print(f'[{sid}] 曲师错误')
            print(f'{conditionArtist}\n{info["composer"]}\n')
            
        if not conditionIllustration:
            print(f'[{sid}] 画师错误')
            print(f'{illustrator}\n{info["illustrator"]}\n')
            
        for index in range(len(info_ratings)):
            level = level_list[index]
            rating_info, rating_db = info_ratings[index], ratings[index]
            note_info, note_db = info_notes[index], notes[index]
            chart_info, chart_db = info_charts[index], charts[index]
            
            if rating_info is not None:
                if not float_equal(rating_info, rating_db):
                    print(f"{title}的{level} 定数: {rating_db} => {rating_info}")
                if note_info != note_db:
                    print(f"{title}的{level} note: {note_db} => {note_info}")
                if chart_info != chart_db:
                    print(f"{title}的{level} 谱师: {chart_db} => {chart_info}")
            
    # 现在，data变量包含了JSON文件的内容，它是一个Python数据结构（如列表或字典）  
    """
    levels = {}
    for key, value in levels.items():
        
        info = info[0]
        compare_level = (info[5], value[0]),(info[6], value[1]),(info[7], value[2]),(info[8], value[3])
    """