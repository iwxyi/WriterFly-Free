#include "shortcutkeymanager.h"

ShortcutKeyManager::ShortcutKeyManager(QString file_path) : file_path(file_path)
{

    QtConcurrent::run(this, &ShortcutKeyManager::initFromFile);

}

QStringList ShortcutKeyManager::getShortutCmds(ShortcutEnvironment env, Qt::KeyboardModifiers modifiers, Qt::Key key)
{
    foreach (ShortcutEntry sc, shortcuts)
    {
        if (sc.env == env && sc.modifier == modifiers && sc.key == key)
            return sc.cmds;
    }
    return QStringList();
}

void ShortcutKeyManager::saveToFile()
{
    QString full = "";
    for (int i = 0; i < shortcuts.size(); i++)
    {
        full += "<SHORTCUT>" + shortcuts.at(i).full_key + "</SHORTCUT>\n";
    }
    writeTextFile(file_path, full);
}

void ShortcutKeyManager::initFromFile()
{
    initKeyMap();
    if (!isFileExist(file_path))
        return ;

    QString content = readTextFileWithFolder(file_path, extra_dir);
    QStringList list = getXmls(content, "SHORTCUT");
    foreach (QString s, list)
    {
        ShortcutEntry sc;
        sc.full_key = s;
        sc.keys = getXml(s, "KEY").trimmed();
        sc.key = NO_KEY;
        sc.modifier = Qt::NoModifier;
        sc.cmds = getXmls(s, "CMD");
        sc.left = getXml(s, "LEFT");
        sc.right = getXml(s, "RIGHT");

        // 读取修饰键
        sc.keys = sc.keys.toLower();
        QStringList key_list = sc.keys.split("+", QString::SkipEmptyParts);
        if (key_list.contains("ctrl"))
            sc.modifier |= Qt::ControlModifier;
        if (key_list.contains("shift"))
            sc.modifier |= Qt::ShiftModifier;
        if (key_list.contains("alt"))
            sc.modifier |= Qt::AltModifier;

        // 读取目标按键
        if (key_list.last() == "ctrl")
            key_list.removeLast();
        if (key_list.last() == "alt")
            key_list.removeLast();
        if (key_list.last() == "shift")
            key_list.removeLast();
        if (!key_list.isEmpty())
            sc.key = KEY_MAP.value(key_list.last(), NO_KEY);

        // 去掉前缀，化为单纯的指令
        for (int i = 0; i < sc.cmds.size(); i++)
        {
            QString& s = sc.cmds[i];
            if (s.startsWith("全局·") || s.startsWith("编辑·")
                    || s.startsWith("大纲·") || s.startsWith("目录·"))
                s = s.right(s.length()-3);
        }

        // 确定作用范围
        QString e = getXml(s, "ENV");
        if (e.isEmpty() || e == "global")
            sc.env = SCK_GLOBAL;
        else if (e == "editor")
            sc.env = SCK_EDITOR;
        else if (e == "editors")
            sc.env = SCK_EDITORS;
        else if (e == "dir")
            sc.env = SCK_DIR;
        else
            sc.env = SCK_GLOBAL;

        shortcuts.append(sc);
    }
}

void ShortcutKeyManager::initKeyMap()
{
    KEY_MAP.insert("esc", Qt::Key_Escape);
    KEY_MAP.insert("\\t", Qt::Key_Tab);
    KEY_MAP.insert("\\n", Qt::Key_Return);
    KEY_MAP.insert("ins", Qt::Key_Insert);
    KEY_MAP.insert("del", Qt::Key_Delete);
    KEY_MAP.insert(" ", Qt::Key_Space);
    KEY_MAP.insert("!", Qt::Key_Exclam);
    KEY_MAP.insert("\"", Qt::Key_QuoteDbl);
    KEY_MAP.insert("#", Qt::Key_NumberSign);
    KEY_MAP.insert("$", Qt::Key_Dollar);
    KEY_MAP.insert("%", Qt::Key_Percent);
    KEY_MAP.insert("&", Qt::Key_Ampersand);
    KEY_MAP.insert("`", Qt::Key_Apostrophe);
    KEY_MAP.insert("(", Qt::Key_ParenLeft);
    KEY_MAP.insert(")", Qt::Key_ParenRight);
    KEY_MAP.insert("*", Qt::Key_Asterisk);
    KEY_MAP.insert("+", Qt::Key_Plus);
    KEY_MAP.insert(",", Qt::Key_Comma);
    KEY_MAP.insert("-", Qt::Key_Minus);
    KEY_MAP.insert(".", Qt::Key_Period);
    KEY_MAP.insert("/", Qt::Key_Slash);
    KEY_MAP.insert("0", Qt::Key_0);
    KEY_MAP.insert("1", Qt::Key_1);
    KEY_MAP.insert("2", Qt::Key_2);
    KEY_MAP.insert("3", Qt::Key_3);
    KEY_MAP.insert("4", Qt::Key_4);
    KEY_MAP.insert("5", Qt::Key_5);
    KEY_MAP.insert("6", Qt::Key_6);
    KEY_MAP.insert("7", Qt::Key_7);
    KEY_MAP.insert("8", Qt::Key_8);
    KEY_MAP.insert("9", Qt::Key_9);
    KEY_MAP.insert(":", Qt::Key_Colon);
    KEY_MAP.insert(";", Qt::Key_Semicolon);
    KEY_MAP.insert("<", Qt::Key_Less);
    KEY_MAP.insert("=", Qt::Key_Equal);
    KEY_MAP.insert(">", Qt::Key_Greater);
    KEY_MAP.insert("?", Qt::Key_Question);
    KEY_MAP.insert("@", Qt::Key_At);
    KEY_MAP.insert("[", Qt::Key_BracketLeft);
    KEY_MAP.insert("\\", Qt::Key_Backslash);
    KEY_MAP.insert("]", Qt::Key_BracketRight);
    KEY_MAP.insert("_", Qt::Key_Underscore);
    KEY_MAP.insert("{", Qt::Key_BraceLeft);
    KEY_MAP.insert("|", Qt::Key_Bar);
    KEY_MAP.insert("}", Qt::Key_BraceRight);
    KEY_MAP.insert("§", Qt::Key_section);

    // ==== 以下为原来的数值 ====

    KEY_MAP.insert("escape", Qt::Key_Escape);
    KEY_MAP.insert("tab", Qt::Key_Tab);
    KEY_MAP.insert("backtab", Qt::Key_Backtab);
    KEY_MAP.insert("backspace", Qt::Key_Backspace);
    KEY_MAP.insert("return", Qt::Key_Return);
    KEY_MAP.insert("enter", Qt::Key_Enter);
    KEY_MAP.insert("insert", Qt::Key_Insert);
    KEY_MAP.insert("delete", Qt::Key_Delete);
    KEY_MAP.insert("pause", Qt::Key_Pause);
    KEY_MAP.insert("print", Qt::Key_Print);
    KEY_MAP.insert("sysreq", Qt::Key_SysReq);
    KEY_MAP.insert("clear", Qt::Key_Clear);
    KEY_MAP.insert("home", Qt::Key_Home);
    KEY_MAP.insert("end", Qt::Key_End);
    KEY_MAP.insert("left", Qt::Key_Left);
    KEY_MAP.insert("up", Qt::Key_Up);
    KEY_MAP.insert("right", Qt::Key_Right);
    KEY_MAP.insert("down", Qt::Key_Down);
    KEY_MAP.insert("pageup", Qt::Key_PageUp);
    KEY_MAP.insert("pagedown", Qt::Key_PageDown);
    KEY_MAP.insert("shift", Qt::Key_Shift);
    KEY_MAP.insert("control", Qt::Key_Control);
    KEY_MAP.insert("meta", Qt::Key_Meta);
    KEY_MAP.insert("alt", Qt::Key_Alt);
    KEY_MAP.insert("altgr", Qt::Key_AltGr);
    KEY_MAP.insert("capslock", Qt::Key_CapsLock);
    KEY_MAP.insert("numlock", Qt::Key_NumLock);
    KEY_MAP.insert("scrolllock", Qt::Key_ScrollLock);
    KEY_MAP.insert("f1", Qt::Key_F1);
    KEY_MAP.insert("f2", Qt::Key_F2);
    KEY_MAP.insert("f3", Qt::Key_F3);
    KEY_MAP.insert("f4", Qt::Key_F4);
    KEY_MAP.insert("f5", Qt::Key_F5);
    KEY_MAP.insert("f6", Qt::Key_F6);
    KEY_MAP.insert("f7", Qt::Key_F7);
    KEY_MAP.insert("f8", Qt::Key_F8);
    KEY_MAP.insert("f9", Qt::Key_F9);
    KEY_MAP.insert("f10", Qt::Key_F10);
    KEY_MAP.insert("f11", Qt::Key_F11);
    KEY_MAP.insert("f12", Qt::Key_F12);
    KEY_MAP.insert("f13", Qt::Key_F13);
    KEY_MAP.insert("f14", Qt::Key_F14);
    KEY_MAP.insert("f15", Qt::Key_F15);
    KEY_MAP.insert("f16", Qt::Key_F16);
    KEY_MAP.insert("f17", Qt::Key_F17);
    KEY_MAP.insert("f18", Qt::Key_F18);
    KEY_MAP.insert("f19", Qt::Key_F19);
    KEY_MAP.insert("f20", Qt::Key_F20);
    KEY_MAP.insert("f21", Qt::Key_F21);
    KEY_MAP.insert("f22", Qt::Key_F22);
    KEY_MAP.insert("f23", Qt::Key_F23);
    KEY_MAP.insert("f24", Qt::Key_F24);
    KEY_MAP.insert("f25", Qt::Key_F25);
    KEY_MAP.insert("f26", Qt::Key_F26);
    KEY_MAP.insert("f27", Qt::Key_F27);
    KEY_MAP.insert("f28", Qt::Key_F28);
    KEY_MAP.insert("f29", Qt::Key_F29);
    KEY_MAP.insert("f30", Qt::Key_F30);
    KEY_MAP.insert("f31", Qt::Key_F31);
    KEY_MAP.insert("f32", Qt::Key_F32);
    KEY_MAP.insert("f33", Qt::Key_F33);
    KEY_MAP.insert("f34", Qt::Key_F34);
    KEY_MAP.insert("f35", Qt::Key_F35);
    KEY_MAP.insert("super_l", Qt::Key_Super_L);
    KEY_MAP.insert("super_r", Qt::Key_Super_R);
    KEY_MAP.insert("menu", Qt::Key_Menu);
    KEY_MAP.insert("hyper_l", Qt::Key_Hyper_L);
    KEY_MAP.insert("hyper_r", Qt::Key_Hyper_R);
    KEY_MAP.insert("help", Qt::Key_Help);
    KEY_MAP.insert("direction_l", Qt::Key_Direction_L);
    KEY_MAP.insert("direction_r", Qt::Key_Direction_R);
    KEY_MAP.insert("space", Qt::Key_Space);
    KEY_MAP.insert("any", Qt::Key_Any);
    KEY_MAP.insert("exclam", Qt::Key_Exclam);
    KEY_MAP.insert("quotedbl", Qt::Key_QuoteDbl);
    KEY_MAP.insert("numbersign", Qt::Key_NumberSign);
    KEY_MAP.insert("dollar", Qt::Key_Dollar);
    KEY_MAP.insert("percent", Qt::Key_Percent);
    KEY_MAP.insert("ampersand", Qt::Key_Ampersand);
    KEY_MAP.insert("apostrophe", Qt::Key_Apostrophe);
    KEY_MAP.insert("parenleft", Qt::Key_ParenLeft);
    KEY_MAP.insert("parenright", Qt::Key_ParenRight);
    KEY_MAP.insert("asterisk", Qt::Key_Asterisk);
    KEY_MAP.insert("plus", Qt::Key_Plus);
    KEY_MAP.insert("comma", Qt::Key_Comma);
    KEY_MAP.insert("minus", Qt::Key_Minus);
    KEY_MAP.insert("period", Qt::Key_Period);
    KEY_MAP.insert("slash", Qt::Key_Slash);
    KEY_MAP.insert("0", Qt::Key_0);
    KEY_MAP.insert("1", Qt::Key_1);
    KEY_MAP.insert("2", Qt::Key_2);
    KEY_MAP.insert("3", Qt::Key_3);
    KEY_MAP.insert("4", Qt::Key_4);
    KEY_MAP.insert("5", Qt::Key_5);
    KEY_MAP.insert("6", Qt::Key_6);
    KEY_MAP.insert("7", Qt::Key_7);
    KEY_MAP.insert("8", Qt::Key_8);
    KEY_MAP.insert("9", Qt::Key_9);
    KEY_MAP.insert("colon", Qt::Key_Colon);
    KEY_MAP.insert("semicolon", Qt::Key_Semicolon);
    KEY_MAP.insert("less", Qt::Key_Less);
    KEY_MAP.insert("equal", Qt::Key_Equal);
    KEY_MAP.insert("greater", Qt::Key_Greater);
    KEY_MAP.insert("question", Qt::Key_Question);
    KEY_MAP.insert("at", Qt::Key_At);
    KEY_MAP.insert("a", Qt::Key_A);
    KEY_MAP.insert("b", Qt::Key_B);
    KEY_MAP.insert("c", Qt::Key_C);
    KEY_MAP.insert("d", Qt::Key_D);
    KEY_MAP.insert("e", Qt::Key_E);
    KEY_MAP.insert("f", Qt::Key_F);
    KEY_MAP.insert("g", Qt::Key_G);
    KEY_MAP.insert("h", Qt::Key_H);
    KEY_MAP.insert("i", Qt::Key_I);
    KEY_MAP.insert("j", Qt::Key_J);
    KEY_MAP.insert("k", Qt::Key_K);
    KEY_MAP.insert("l", Qt::Key_L);
    KEY_MAP.insert("m", Qt::Key_M);
    KEY_MAP.insert("n", Qt::Key_N);
    KEY_MAP.insert("o", Qt::Key_O);
    KEY_MAP.insert("p", Qt::Key_P);
    KEY_MAP.insert("q", Qt::Key_Q);
    KEY_MAP.insert("r", Qt::Key_R);
    KEY_MAP.insert("s", Qt::Key_S);
    KEY_MAP.insert("t", Qt::Key_T);
    KEY_MAP.insert("u", Qt::Key_U);
    KEY_MAP.insert("v", Qt::Key_V);
    KEY_MAP.insert("w", Qt::Key_W);
    KEY_MAP.insert("x", Qt::Key_X);
    KEY_MAP.insert("y", Qt::Key_Y);
    KEY_MAP.insert("z", Qt::Key_Z);
    KEY_MAP.insert("bracketleft", Qt::Key_BracketLeft);
    KEY_MAP.insert("backslash", Qt::Key_Backslash);
    KEY_MAP.insert("bracketright", Qt::Key_BracketRight);
    KEY_MAP.insert("asciicircum", Qt::Key_AsciiCircum);
    KEY_MAP.insert("underscore", Qt::Key_Underscore);
    KEY_MAP.insert("quoteleft", Qt::Key_QuoteLeft);
    KEY_MAP.insert("braceleft", Qt::Key_BraceLeft);
    KEY_MAP.insert("bar", Qt::Key_Bar);
    KEY_MAP.insert("braceright", Qt::Key_BraceRight);
    KEY_MAP.insert("asciitilde", Qt::Key_AsciiTilde);
    KEY_MAP.insert("nobreakspace", Qt::Key_nobreakspace);
    KEY_MAP.insert("exclamdown", Qt::Key_exclamdown);
    KEY_MAP.insert("cent", Qt::Key_cent);
    KEY_MAP.insert("sterling", Qt::Key_sterling);
    KEY_MAP.insert("currency", Qt::Key_currency);
    KEY_MAP.insert("yen", Qt::Key_yen);
    KEY_MAP.insert("brokenbar", Qt::Key_brokenbar);
    KEY_MAP.insert("section", Qt::Key_section);
    KEY_MAP.insert("diaeresis", Qt::Key_diaeresis);
    KEY_MAP.insert("copyright", Qt::Key_copyright);
    KEY_MAP.insert("ordfeminine", Qt::Key_ordfeminine);
    KEY_MAP.insert("guillemotleft", Qt::Key_guillemotleft);
    KEY_MAP.insert("notsign", Qt::Key_notsign);
    KEY_MAP.insert("hyphen", Qt::Key_hyphen);
    KEY_MAP.insert("registered", Qt::Key_registered);
    KEY_MAP.insert("macron", Qt::Key_macron);
    KEY_MAP.insert("degree", Qt::Key_degree);
    KEY_MAP.insert("plusminus", Qt::Key_plusminus);
    KEY_MAP.insert("twosuperior", Qt::Key_twosuperior);
    KEY_MAP.insert("threesuperior", Qt::Key_threesuperior);
    KEY_MAP.insert("acute", Qt::Key_acute);
    KEY_MAP.insert("mu", Qt::Key_mu);
    KEY_MAP.insert("paragraph", Qt::Key_paragraph);
    KEY_MAP.insert("periodcentered", Qt::Key_periodcentered);
    KEY_MAP.insert("cedilla", Qt::Key_cedilla);
    KEY_MAP.insert("onesuperior", Qt::Key_onesuperior);
    KEY_MAP.insert("masculine", Qt::Key_masculine);
    KEY_MAP.insert("guillemotright", Qt::Key_guillemotright);
    KEY_MAP.insert("onequarter", Qt::Key_onequarter);
    KEY_MAP.insert("onehalf", Qt::Key_onehalf);
    KEY_MAP.insert("threequarters", Qt::Key_threequarters);
    KEY_MAP.insert("questiondown", Qt::Key_questiondown);
    KEY_MAP.insert("agrave", Qt::Key_Agrave);
    KEY_MAP.insert("aacute", Qt::Key_Aacute);
    KEY_MAP.insert("acircumflex", Qt::Key_Acircumflex);
    KEY_MAP.insert("atilde", Qt::Key_Atilde);
    KEY_MAP.insert("adiaeresis", Qt::Key_Adiaeresis);
    KEY_MAP.insert("aring", Qt::Key_Aring);
    KEY_MAP.insert("ae", Qt::Key_AE);
    KEY_MAP.insert("ccedilla", Qt::Key_Ccedilla);
    KEY_MAP.insert("egrave", Qt::Key_Egrave);
    KEY_MAP.insert("eacute", Qt::Key_Eacute);
    KEY_MAP.insert("ecircumflex", Qt::Key_Ecircumflex);
    KEY_MAP.insert("ediaeresis", Qt::Key_Ediaeresis);
    KEY_MAP.insert("igrave", Qt::Key_Igrave);
    KEY_MAP.insert("iacute", Qt::Key_Iacute);
    KEY_MAP.insert("icircumflex", Qt::Key_Icircumflex);
    KEY_MAP.insert("idiaeresis", Qt::Key_Idiaeresis);
    KEY_MAP.insert("eth", Qt::Key_ETH);
    KEY_MAP.insert("ntilde", Qt::Key_Ntilde);
    KEY_MAP.insert("ograve", Qt::Key_Ograve);
    KEY_MAP.insert("oacute", Qt::Key_Oacute);
    KEY_MAP.insert("ocircumflex", Qt::Key_Ocircumflex);
    KEY_MAP.insert("otilde", Qt::Key_Otilde);
    KEY_MAP.insert("odiaeresis", Qt::Key_Odiaeresis);
    KEY_MAP.insert("multiply", Qt::Key_multiply);
    KEY_MAP.insert("ooblique", Qt::Key_Ooblique);
    KEY_MAP.insert("ugrave", Qt::Key_Ugrave);
    KEY_MAP.insert("uacute", Qt::Key_Uacute);
    KEY_MAP.insert("ucircumflex", Qt::Key_Ucircumflex);
    KEY_MAP.insert("udiaeresis", Qt::Key_Udiaeresis);
    KEY_MAP.insert("yacute", Qt::Key_Yacute);
    KEY_MAP.insert("thorn", Qt::Key_THORN);
    KEY_MAP.insert("ssharp", Qt::Key_ssharp);
    KEY_MAP.insert("division", Qt::Key_division);
    KEY_MAP.insert("ydiaeresis", Qt::Key_ydiaeresis);
    KEY_MAP.insert("multi_key", Qt::Key_Multi_key);
    KEY_MAP.insert("codeinput", Qt::Key_Codeinput);
    KEY_MAP.insert("singlecandidate", Qt::Key_SingleCandidate);
    KEY_MAP.insert("multiplecandidate", Qt::Key_MultipleCandidate);
    KEY_MAP.insert("previouscandidate", Qt::Key_PreviousCandidate);
    KEY_MAP.insert("mode_switch", Qt::Key_Mode_switch);
    KEY_MAP.insert("kanji", Qt::Key_Kanji);
    KEY_MAP.insert("muhenkan", Qt::Key_Muhenkan);
    KEY_MAP.insert("henkan", Qt::Key_Henkan);
    KEY_MAP.insert("romaji", Qt::Key_Romaji);
    KEY_MAP.insert("hiragana", Qt::Key_Hiragana);
    KEY_MAP.insert("katakana", Qt::Key_Katakana);
    KEY_MAP.insert("hiragana_katakana", Qt::Key_Hiragana_Katakana);
    KEY_MAP.insert("zenkaku", Qt::Key_Zenkaku);
    KEY_MAP.insert("hankaku", Qt::Key_Hankaku);
    KEY_MAP.insert("zenkaku_hankaku", Qt::Key_Zenkaku_Hankaku);
    KEY_MAP.insert("touroku", Qt::Key_Touroku);
    KEY_MAP.insert("massyo", Qt::Key_Massyo);
    KEY_MAP.insert("kana_lock", Qt::Key_Kana_Lock);
    KEY_MAP.insert("kana_shift", Qt::Key_Kana_Shift);
    KEY_MAP.insert("eisu_shift", Qt::Key_Eisu_Shift);
    KEY_MAP.insert("eisu_toggle", Qt::Key_Eisu_toggle);
    KEY_MAP.insert("hangul", Qt::Key_Hangul);
    KEY_MAP.insert("hangul_start", Qt::Key_Hangul_Start);
    KEY_MAP.insert("hangul_end", Qt::Key_Hangul_End);
    KEY_MAP.insert("hangul_hanja", Qt::Key_Hangul_Hanja);
    KEY_MAP.insert("hangul_jamo", Qt::Key_Hangul_Jamo);
    KEY_MAP.insert("hangul_romaja", Qt::Key_Hangul_Romaja);
    KEY_MAP.insert("hangul_jeonja", Qt::Key_Hangul_Jeonja);
    KEY_MAP.insert("hangul_banja", Qt::Key_Hangul_Banja);
    KEY_MAP.insert("hangul_prehanja", Qt::Key_Hangul_PreHanja);
    KEY_MAP.insert("hangul_posthanja", Qt::Key_Hangul_PostHanja);
    KEY_MAP.insert("hangul_special", Qt::Key_Hangul_Special);
    KEY_MAP.insert("dead_grave", Qt::Key_Dead_Grave);
    KEY_MAP.insert("dead_acute", Qt::Key_Dead_Acute);
    KEY_MAP.insert("dead_circumflex", Qt::Key_Dead_Circumflex);
    KEY_MAP.insert("dead_tilde", Qt::Key_Dead_Tilde);
    KEY_MAP.insert("dead_macron", Qt::Key_Dead_Macron);
    KEY_MAP.insert("dead_breve", Qt::Key_Dead_Breve);
    KEY_MAP.insert("dead_abovedot", Qt::Key_Dead_Abovedot);
    KEY_MAP.insert("dead_diaeresis", Qt::Key_Dead_Diaeresis);
    KEY_MAP.insert("dead_abovering", Qt::Key_Dead_Abovering);
    KEY_MAP.insert("dead_doubleacute", Qt::Key_Dead_Doubleacute);
    KEY_MAP.insert("dead_caron", Qt::Key_Dead_Caron);
    KEY_MAP.insert("dead_cedilla", Qt::Key_Dead_Cedilla);
    KEY_MAP.insert("dead_ogonek", Qt::Key_Dead_Ogonek);
    KEY_MAP.insert("dead_iota", Qt::Key_Dead_Iota);
    KEY_MAP.insert("dead_voiced_sound", Qt::Key_Dead_Voiced_Sound);
    KEY_MAP.insert("dead_semivoiced_sound", Qt::Key_Dead_Semivoiced_Sound);
    KEY_MAP.insert("dead_belowdot", Qt::Key_Dead_Belowdot);
    KEY_MAP.insert("dead_hook", Qt::Key_Dead_Hook);
    KEY_MAP.insert("dead_horn", Qt::Key_Dead_Horn);
    KEY_MAP.insert("dead_stroke", Qt::Key_Dead_Stroke);
    KEY_MAP.insert("dead_abovecomma", Qt::Key_Dead_Abovecomma);
    KEY_MAP.insert("dead_abovereversedcomma", Qt::Key_Dead_Abovereversedcomma);
    KEY_MAP.insert("dead_doublegrave", Qt::Key_Dead_Doublegrave);
    KEY_MAP.insert("dead_belowring", Qt::Key_Dead_Belowring);
    KEY_MAP.insert("dead_belowmacron", Qt::Key_Dead_Belowmacron);
    KEY_MAP.insert("dead_belowcircumflex", Qt::Key_Dead_Belowcircumflex);
    KEY_MAP.insert("dead_belowtilde", Qt::Key_Dead_Belowtilde);
    KEY_MAP.insert("dead_belowbreve", Qt::Key_Dead_Belowbreve);
    KEY_MAP.insert("dead_belowdiaeresis", Qt::Key_Dead_Belowdiaeresis);
    KEY_MAP.insert("dead_invertedbreve", Qt::Key_Dead_Invertedbreve);
    KEY_MAP.insert("dead_belowcomma", Qt::Key_Dead_Belowcomma);
    KEY_MAP.insert("dead_currency", Qt::Key_Dead_Currency);
    KEY_MAP.insert("dead_a", Qt::Key_Dead_a);
    KEY_MAP.insert("dead_a", Qt::Key_Dead_A);
    KEY_MAP.insert("dead_e", Qt::Key_Dead_e);
    KEY_MAP.insert("dead_e", Qt::Key_Dead_E);
    KEY_MAP.insert("dead_i", Qt::Key_Dead_i);
    KEY_MAP.insert("dead_i", Qt::Key_Dead_I);
    KEY_MAP.insert("dead_o", Qt::Key_Dead_o);
    KEY_MAP.insert("dead_o", Qt::Key_Dead_O);
    KEY_MAP.insert("dead_u", Qt::Key_Dead_u);
    KEY_MAP.insert("dead_u", Qt::Key_Dead_U);
    KEY_MAP.insert("dead_small_schwa", Qt::Key_Dead_Small_Schwa);
    KEY_MAP.insert("dead_capital_schwa", Qt::Key_Dead_Capital_Schwa);
    KEY_MAP.insert("dead_greek", Qt::Key_Dead_Greek);
    KEY_MAP.insert("dead_lowline", Qt::Key_Dead_Lowline);
    KEY_MAP.insert("dead_aboveverticalline", Qt::Key_Dead_Aboveverticalline);
    KEY_MAP.insert("dead_belowverticalline", Qt::Key_Dead_Belowverticalline);
    KEY_MAP.insert("dead_longsolidusoverlay", Qt::Key_Dead_Longsolidusoverlay);
    KEY_MAP.insert("back", Qt::Key_Back);
    KEY_MAP.insert("forward", Qt::Key_Forward);
    KEY_MAP.insert("stop", Qt::Key_Stop);
    KEY_MAP.insert("refresh", Qt::Key_Refresh);
    KEY_MAP.insert("volumedown", Qt::Key_VolumeDown);
    KEY_MAP.insert("volumemute", Qt::Key_VolumeMute);
    KEY_MAP.insert("volumeup", Qt::Key_VolumeUp);
    KEY_MAP.insert("bassboost", Qt::Key_BassBoost);
    KEY_MAP.insert("bassup", Qt::Key_BassUp);
    KEY_MAP.insert("bassdown", Qt::Key_BassDown);
    KEY_MAP.insert("trebleup", Qt::Key_TrebleUp);
    KEY_MAP.insert("trebledown", Qt::Key_TrebleDown);
    KEY_MAP.insert("mediaplay", Qt::Key_MediaPlay);
    KEY_MAP.insert("mediastop", Qt::Key_MediaStop);
    KEY_MAP.insert("mediaprevious", Qt::Key_MediaPrevious);
    KEY_MAP.insert("medianext", Qt::Key_MediaNext);
    KEY_MAP.insert("mediarecord", Qt::Key_MediaRecord);
    KEY_MAP.insert("mediapause", Qt::Key_MediaPause);
    KEY_MAP.insert("mediatoggleplaypause", Qt::Key_MediaTogglePlayPause);
    KEY_MAP.insert("homepage", Qt::Key_HomePage);
    KEY_MAP.insert("favorites", Qt::Key_Favorites);
    KEY_MAP.insert("search", Qt::Key_Search);
    KEY_MAP.insert("standby", Qt::Key_Standby);
    KEY_MAP.insert("openurl", Qt::Key_OpenUrl);
    KEY_MAP.insert("launchmail", Qt::Key_LaunchMail);
    KEY_MAP.insert("launchmedia", Qt::Key_LaunchMedia);
    KEY_MAP.insert("launch0", Qt::Key_Launch0);
    KEY_MAP.insert("launch1", Qt::Key_Launch1);
    KEY_MAP.insert("launch2", Qt::Key_Launch2);
    KEY_MAP.insert("launch3", Qt::Key_Launch3);
    KEY_MAP.insert("launch4", Qt::Key_Launch4);
    KEY_MAP.insert("launch5", Qt::Key_Launch5);
    KEY_MAP.insert("launch6", Qt::Key_Launch6);
    KEY_MAP.insert("launch7", Qt::Key_Launch7);
    KEY_MAP.insert("launch8", Qt::Key_Launch8);
    KEY_MAP.insert("launch9", Qt::Key_Launch9);
    KEY_MAP.insert("launcha", Qt::Key_LaunchA);
    KEY_MAP.insert("launchb", Qt::Key_LaunchB);
    KEY_MAP.insert("launchc", Qt::Key_LaunchC);
    KEY_MAP.insert("launchd", Qt::Key_LaunchD);
    KEY_MAP.insert("launche", Qt::Key_LaunchE);
    KEY_MAP.insert("launchf", Qt::Key_LaunchF);
    KEY_MAP.insert("launchg", Qt::Key_LaunchG);
    KEY_MAP.insert("launchh", Qt::Key_LaunchH);
    KEY_MAP.insert("monbrightnessup", Qt::Key_MonBrightnessUp);
    KEY_MAP.insert("monbrightnessdown", Qt::Key_MonBrightnessDown);
    KEY_MAP.insert("keyboardlightonoff", Qt::Key_KeyboardLightOnOff);
    KEY_MAP.insert("keyboardbrightnessup", Qt::Key_KeyboardBrightnessUp);
    KEY_MAP.insert("keyboardbrightnessdown", Qt::Key_KeyboardBrightnessDown);
    KEY_MAP.insert("poweroff", Qt::Key_PowerOff);
    KEY_MAP.insert("wakeup", Qt::Key_WakeUp);
    KEY_MAP.insert("eject", Qt::Key_Eject);
    KEY_MAP.insert("screensaver", Qt::Key_ScreenSaver);
    KEY_MAP.insert("www", Qt::Key_WWW);
    KEY_MAP.insert("memo", Qt::Key_Memo);
    KEY_MAP.insert("lightbulb", Qt::Key_LightBulb);
    KEY_MAP.insert("shop", Qt::Key_Shop);
    KEY_MAP.insert("history", Qt::Key_History);
    KEY_MAP.insert("addfavorite", Qt::Key_AddFavorite);
    KEY_MAP.insert("hotlinks", Qt::Key_HotLinks);
    KEY_MAP.insert("brightnessadjust", Qt::Key_BrightnessAdjust);
    KEY_MAP.insert("finance", Qt::Key_Finance);
    KEY_MAP.insert("community", Qt::Key_Community);
    KEY_MAP.insert("audiorewind", Qt::Key_AudioRewind);
    KEY_MAP.insert("backforward", Qt::Key_BackForward);
    KEY_MAP.insert("applicationleft", Qt::Key_ApplicationLeft);
    KEY_MAP.insert("applicationright", Qt::Key_ApplicationRight);
    KEY_MAP.insert("book", Qt::Key_Book);
    KEY_MAP.insert("cd", Qt::Key_CD);
    KEY_MAP.insert("calculator", Qt::Key_Calculator);
    KEY_MAP.insert("todolist", Qt::Key_ToDoList);
    KEY_MAP.insert("cleargrab", Qt::Key_ClearGrab);
    KEY_MAP.insert("close", Qt::Key_Close);
    KEY_MAP.insert("copy", Qt::Key_Copy);
    KEY_MAP.insert("cut", Qt::Key_Cut);
    KEY_MAP.insert("display", Qt::Key_Display);
    KEY_MAP.insert("dos", Qt::Key_DOS);
    KEY_MAP.insert("documents", Qt::Key_Documents);
    KEY_MAP.insert("excel", Qt::Key_Excel);
    KEY_MAP.insert("explorer", Qt::Key_Explorer);
    KEY_MAP.insert("game", Qt::Key_Game);
    KEY_MAP.insert("go", Qt::Key_Go);
    KEY_MAP.insert("itouch", Qt::Key_iTouch);
    KEY_MAP.insert("logoff", Qt::Key_LogOff);
    KEY_MAP.insert("market", Qt::Key_Market);
    KEY_MAP.insert("meeting", Qt::Key_Meeting);
    KEY_MAP.insert("menukb", Qt::Key_MenuKB);
    KEY_MAP.insert("menupb", Qt::Key_MenuPB);
    KEY_MAP.insert("mysites", Qt::Key_MySites);
    KEY_MAP.insert("news", Qt::Key_News);
    KEY_MAP.insert("officehome", Qt::Key_OfficeHome);
    KEY_MAP.insert("option", Qt::Key_Option);
    KEY_MAP.insert("paste", Qt::Key_Paste);
    KEY_MAP.insert("phone", Qt::Key_Phone);
    KEY_MAP.insert("calendar", Qt::Key_Calendar);
    KEY_MAP.insert("reply", Qt::Key_Reply);
    KEY_MAP.insert("reload", Qt::Key_Reload);
    KEY_MAP.insert("rotatewindows", Qt::Key_RotateWindows);
    KEY_MAP.insert("rotationpb", Qt::Key_RotationPB);
    KEY_MAP.insert("rotationkb", Qt::Key_RotationKB);
    KEY_MAP.insert("save", Qt::Key_Save);
    KEY_MAP.insert("send", Qt::Key_Send);
    KEY_MAP.insert("spell", Qt::Key_Spell);
    KEY_MAP.insert("splitscreen", Qt::Key_SplitScreen);
    KEY_MAP.insert("support", Qt::Key_Support);
    KEY_MAP.insert("taskpane", Qt::Key_TaskPane);
    KEY_MAP.insert("terminal", Qt::Key_Terminal);
    KEY_MAP.insert("tools", Qt::Key_Tools);
    KEY_MAP.insert("travel", Qt::Key_Travel);
    KEY_MAP.insert("video", Qt::Key_Video);
    KEY_MAP.insert("word", Qt::Key_Word);
    KEY_MAP.insert("xfer", Qt::Key_Xfer);
    KEY_MAP.insert("zoomin", Qt::Key_ZoomIn);
    KEY_MAP.insert("zoomout", Qt::Key_ZoomOut);
    KEY_MAP.insert("away", Qt::Key_Away);
    KEY_MAP.insert("messenger", Qt::Key_Messenger);
    KEY_MAP.insert("webcam", Qt::Key_WebCam);
    KEY_MAP.insert("mailforward", Qt::Key_MailForward);
    KEY_MAP.insert("pictures", Qt::Key_Pictures);
    KEY_MAP.insert("music", Qt::Key_Music);
    KEY_MAP.insert("battery", Qt::Key_Battery);
    KEY_MAP.insert("bluetooth", Qt::Key_Bluetooth);
    KEY_MAP.insert("wlan", Qt::Key_WLAN);
    KEY_MAP.insert("uwb", Qt::Key_UWB);
    KEY_MAP.insert("audioforward", Qt::Key_AudioForward);
    KEY_MAP.insert("audiorepeat", Qt::Key_AudioRepeat);
    KEY_MAP.insert("audiorandomplay", Qt::Key_AudioRandomPlay);
    KEY_MAP.insert("subtitle", Qt::Key_Subtitle);
    KEY_MAP.insert("audiocycletrack", Qt::Key_AudioCycleTrack);
    KEY_MAP.insert("time", Qt::Key_Time);
    KEY_MAP.insert("hibernate", Qt::Key_Hibernate);
    KEY_MAP.insert("view", Qt::Key_View);
    KEY_MAP.insert("topmenu", Qt::Key_TopMenu);
    KEY_MAP.insert("powerdown", Qt::Key_PowerDown);
    KEY_MAP.insert("suspend", Qt::Key_Suspend);
    KEY_MAP.insert("contrastadjust", Qt::Key_ContrastAdjust);
    KEY_MAP.insert("touchpadtoggle", Qt::Key_TouchpadToggle);
    KEY_MAP.insert("touchpadon", Qt::Key_TouchpadOn);
    KEY_MAP.insert("touchpadoff", Qt::Key_TouchpadOff);
    KEY_MAP.insert("micmute", Qt::Key_MicMute);
    KEY_MAP.insert("red", Qt::Key_Red);
    KEY_MAP.insert("green", Qt::Key_Green);
    KEY_MAP.insert("yellow", Qt::Key_Yellow);
    KEY_MAP.insert("blue", Qt::Key_Blue);
    KEY_MAP.insert("channelup", Qt::Key_ChannelUp);
    KEY_MAP.insert("channeldown", Qt::Key_ChannelDown);
    KEY_MAP.insert("guide", Qt::Key_Guide);
    KEY_MAP.insert("info", Qt::Key_Info);
    KEY_MAP.insert("settings", Qt::Key_Settings);
    KEY_MAP.insert("micvolumeup", Qt::Key_MicVolumeUp);
    KEY_MAP.insert("micvolumedown", Qt::Key_MicVolumeDown);
    KEY_MAP.insert("new", Qt::Key_New);
    KEY_MAP.insert("open", Qt::Key_Open);
    KEY_MAP.insert("find", Qt::Key_Find);
    KEY_MAP.insert("undo", Qt::Key_Undo);
    KEY_MAP.insert("redo", Qt::Key_Redo);
    KEY_MAP.insert("medialast", Qt::Key_MediaLast);
    KEY_MAP.insert("unknown", Qt::Key_unknown);
    KEY_MAP.insert("call", Qt::Key_Call);
    KEY_MAP.insert("camera", Qt::Key_Camera);
    KEY_MAP.insert("camerafocus", Qt::Key_CameraFocus);
    KEY_MAP.insert("context1", Qt::Key_Context1);
    KEY_MAP.insert("context2", Qt::Key_Context2);
    KEY_MAP.insert("context3", Qt::Key_Context3);
    KEY_MAP.insert("context4", Qt::Key_Context4);
    KEY_MAP.insert("flip", Qt::Key_Flip);
    KEY_MAP.insert("hangup", Qt::Key_Hangup);
    KEY_MAP.insert("no", Qt::Key_No);
    KEY_MAP.insert("select", Qt::Key_Select);
    KEY_MAP.insert("yes", Qt::Key_Yes);
    KEY_MAP.insert("togglecallhangup", Qt::Key_ToggleCallHangup);
    KEY_MAP.insert("voicedial", Qt::Key_VoiceDial);
    KEY_MAP.insert("lastnumberredial", Qt::Key_LastNumberRedial);
    KEY_MAP.insert("execute", Qt::Key_Execute);
    KEY_MAP.insert("printer", Qt::Key_Printer);
    KEY_MAP.insert("play", Qt::Key_Play);
    KEY_MAP.insert("sleep", Qt::Key_Sleep);
    KEY_MAP.insert("zoom", Qt::Key_Zoom);
    KEY_MAP.insert("exit", Qt::Key_Exit);
    KEY_MAP.insert("cancel", Qt::Key_Cancel);
}
