idf_component_register(
        SRCS 
            "Irrigation_IoT.c"
            "Driver/BH1750FVI.c"
            "Driver/SHT30.c"
            "Driver/SEN0193.c"
            "Network/Wifi.c"
            "Network/Mqtt.c"
            "Application/Controller.c"
        INCLUDE_DIRS 
            "."
            "Driver"
            "Network"
            "Security "
            "Application"
        EMBED_TXTFILES
            "Security/CA.pem")
