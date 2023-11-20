def test_get_app_and_version(cmd):
    # for now, Speculos always returns app:1.33.7, so this test is not very meaningfull
    # however, its failure will mean that Speculos may have been improved on this front
    app_name, version = cmd.get_app_and_version()

    assert app_name == "Phantasma"
    assert version == "1.33.7"
