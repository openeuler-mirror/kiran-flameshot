# 翻译目录（catalog）。
#
# 调用方必须已经找到 Qt5LinguistTools。下面的列表是 translations/ 中随源码提供的
# 全部 .ts catalog，全部参与编译并安装。
set(KFS_TS_FILES
    translations/Internationalization_bo_CN.ts
    translations/Internationalization_ca.ts
    translations/Internationalization_de_DE.ts
    translations/Internationalization_es.ts
    translations/Internationalization_fr.ts
    translations/Internationalization_hu.ts
    translations/Internationalization_ja.ts
    translations/Internationalization_ka.ts
    translations/Internationalization_kk_KG.ts
    translations/Internationalization_kk_KZ.ts
    translations/Internationalization_mn_MN.ts
    translations/Internationalization_nl.ts
    translations/Internationalization_pl.ts
    translations/Internationalization_pt_br.ts
    translations/Internationalization_ru.ts
    translations/Internationalization_sk.ts
    translations/Internationalization_sr.ts
    translations/Internationalization_tr.ts
    translations/Internationalization_ug_CN.ts
    translations/Internationalization_uk.ts
    translations/Internationalization_zh_CN.ts
    translations/Internationalization_zh_TW.ts)

foreach(ts_file IN LISTS KFS_TS_FILES)
    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/${ts_file}")
        message(FATAL_ERROR "missing translation source: ${ts_file}")
    endif()
endforeach()

qt5_add_translation(KFS_QM_FILES ${KFS_TS_FILES})
add_custom_target(kfs-translations ALL DEPENDS ${KFS_QM_FILES})
