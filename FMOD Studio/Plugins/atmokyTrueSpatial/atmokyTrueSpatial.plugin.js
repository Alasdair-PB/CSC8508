studio.plugins.registerPluginDescription("atmoky Spatializer", {
    companyName: "atmoky",
    productName: "atmoky Spatializer",
    parameters: {
        outputFormat: { displayName: "Output Format" },
        gain: { displayName: "Gain" },
        lfeGain: { displayName: "LFE" },
        width: { displayName: "Width" },
        innerAngle: { displayName: "Inner Angle" },
        outerAngle: { displayName: "Outer Angle" },
        outerGain: { displayName: "Gain" },
        outerLowpass: { displayName: "Lowpass" },
        occlusion: { displayName: "Occlusion" },
        nfeDistance: { displayName: "Distance" },
        nfeGain: { displayName: "Gain" },
        nfeBassBoost: { displayName: "Bass Boost" },
        binauralIf2ch: { displayName: "Platform Options" },
    },
    defaultConfigurations: [
        {
            name: "Omni",
            parameterValues: {
                innerAngle: 360,
                outerAngle: 360,
                outerGain: 0,
                outerLowpass: 0,
            },
        },
        {
            name: "Cardioid",
            parameterValues: {
                innerAngle: 0,
                outerAngle: 360,
                outerGain: -80,
                outerLowpass: 0,
            },
        },
        {
            name: "Voice",
            parameterValues: {
                innerAngle: 130,
                outerAngle: 300,
                outerGain: -3,
                outerLowpass: 0.4,
            },
        },
        {
            name: "Loudspeaker",
            parameterValues: {
                innerAngle: 90,
                outerAngle: 360,
                outerGain: -4,
                outerLowpass: 0.3,
            },
        },
        {
            name: "Narrow Beam",
            parameterValues: {
                innerAngle: 45,
                outerAngle: 180,
                outerGain: -60,
                outerLowpass: 1,
            },
        },
        {
            name: "Wide Beam",
            parameterValues: {
                innerAngle: 60,
                outerAngle: 270,
                outerGain: -20,
                outerLowpass: 0.5,
            },
        },
    ],
    deckUi: {
        deckWidgetType: studio.ui.deckWidgetType.Layout,
        layout: studio.ui.layoutType.HBoxLayout,
        spacing: 10,
        items: [
            {
                deckWidgetType: studio.ui.deckWidgetType.InputMeter,
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
                minimumWidth: 160,
                alignment: studio.ui.alignment.AlignTop,
                spacing: 20,
                items: [
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Pixmap,
                        filePath: __dirname + "/atmokyTrueSpatialLogo.png",
                    },
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Layout,
                        alignment: studio.ui.alignment.AlignBottom,
                        layout: studio.ui.layoutType.HBoxLayout,
                        isFramed: true,
                        spacing: 10,
                        items: [
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Dial,
                                color: "#9900FF",
                                binding: "gain",
                            },
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Layout,
                                layout: studio.ui.layoutType.VBoxLayout,
                                spacing: 8,
                                items: [
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Dropdown,
                                        alignment: studio.ui.alignment.AlignCenter,
                                        binding: "outputFormat",
                                    },
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Button,
                                        buttonWidth: 100,
                                        text: "2Ch to Binaural",
                                        binding: "binauralIf2ch",
                                    },
                                ],
                            },
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Spacer,
                                maximumWidth: 3,
                            },
                        ],
                    },
                ],
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
                minimumWidth: 128,
                maximumWidth: 250,
                spacing: 8,
                contentsMargins: { left: 4, right: 4 },
                alignment: studio.ui.alignment.AlignTop,
                isFramed: true,
                items: [
                    {
                        deckWidgetType: studio.ui.deckWidgetType.DistanceRolloffGraph,
                        rolloffTypeBinding: "DistRolloff",
                        minimumDistanceBinding: "MinDistance",
                        maximumDistanceBinding: "MaxDistance",
                        rolloffTypes: {
                            0: studio.project.distanceRolloffType.LinearSquared,
                            1: studio.project.distanceRolloffType.Linear,
                            2: studio.project.distanceRolloffType.Inverse,
                            3: studio.project.distanceRolloffType.InverseTapered,
                            4: studio.project.distanceRolloffType.Custom,
                        },
                    },
                    {
                        deckWidgetType: studio.ui.deckWidgetType.MinMaxFader,
                        text: "Min & Max Distances",
                        minimumBinding: "MinDistance",
                        maximumBinding: "MaxDistance",
                    },
                ],
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
                spacing: 13,
                isFramed: true,
                items: [
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Dial,
                        color: "#9900FF",
                        binding: "width",
                    },
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Dial,
                        color: "#9900FF",
                        binding: "occlusion",
                    },
                ],
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
                spacing: 10,
                items: [
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Layout,
                        layout: studio.ui.layoutType.VBoxLayout,
                        isFramed: true,
                        minimumWidth: 128,
                        items: [
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Label,
                                text: "Near-field Effects",
                            },
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Dial,
                                color: "#9900FF",
                                binding: "nfeDistance",
                            },
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Layout,
                                layout: studio.ui.layoutType.HBoxLayout,
                                items: [
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Dial,
                                        color: "#9900FF",
                                        binding: "nfeGain",
                                    },
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Dial,
                                        color: "#9900FF",
                                        binding: "nfeBassBoost",
                                    },
                                ],
                            },
                        ],
                    },
                ],
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
                spacing: 10,
                items: [
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Layout,
                        layout: studio.ui.layoutType.VBoxLayout,
                        isFramed: true,
                        minimumWidth: 128,
                        items: [
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Label,
                                text: "Directivity",
                            },
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Layout,
                                layout: studio.ui.layoutType.GridLayout,
                                items: [
                                    {
                                        row: 0,
                                        column: 0,
                                        deckWidgetType: studio.ui.deckWidgetType.Dial,
                                        color: "#9900FF",
                                        binding: "outerGain",
                                    },
                                    {
                                        row: 0,
                                        column: 1,
                                        deckWidgetType: studio.ui.deckWidgetType.Dial,
                                        color: "#9900FF",
                                        binding: "outerLowpass",
                                    },
                                    {
                                        row: 1,
                                        column: 0,
                                        deckWidgetType: studio.ui.deckWidgetType.Dial,
                                        color: "#9900FF",
                                        binding: "innerAngle",
                                    },
                                    {
                                        row: 1,
                                        column: 1,
                                        deckWidgetType: studio.ui.deckWidgetType.Dial,
                                        color: "#9900FF",
                                        binding: "outerAngle",
                                    },
                                ],
                            },
                        ],
                    },
                ],
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.Fader,
                binding: "lfeGain",
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.OutputMeter,
            },
        ],
    },
});

studio.plugins.registerPluginDescription("atmoky Renderer", {
    companyName: "atmoky",
    productName: "atmoky Renderer",
    parameters: {
        outputFormat: { displayName: "Output Format" },
        binauralIf2ch: { displayName: "Platform Options" },
        heightFilter: { displayName: "Height Filter" },
        rearFilter: { displayName: "Rear Filter" },
    },
    deckUi: {
        deckWidgetType: studio.ui.deckWidgetType.Layout,
        layout: studio.ui.layoutType.HBoxLayout,
        spacing: 10,
        items: [
            {
                deckWidgetType: studio.ui.deckWidgetType.InputMeter,
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
                minimumWidth: 160,
                alignment: studio.ui.alignment.AlignTop,
                spacing: 20,
                items: [
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Pixmap,
                        filePath: __dirname + "/atmokyTrueSpatialLogo.png",
                    },
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Layout,
                        alignment: studio.ui.alignment.AlignBottom,
                        layout: studio.ui.layoutType.HBoxLayout,
                        isFramed: true,
                        spacing: 10,
                        items: [
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Layout,
                                layout: studio.ui.layoutType.VBoxLayout,
                                spacing: 8,
                                items: [
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Dropdown,
                                        alignment: studio.ui.alignment.AlignCenter,
                                        binding: "outputFormat",
                                    },
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Button,
                                        buttonWidth: 100,
                                        text: "2Ch to Binaural",
                                        binding: "binauralIf2ch",
                                    },
                                ],
                            },
                        ],
                    },
                ],
            },

            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
                spacing: 13,
                isFramed: true,
                items: [
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Label,
                        text: "Down-mixing options",
                    },
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Layout,
                        layout: studio.ui.layoutType.HBoxLayout,
                        spacing: 13,
                        contentsMargins: { left: 10, right: 10 },
                        items: [
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Dial,
                                color: "#9900FF",
                                binding: "heightFilter",
                            },
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Dial,
                                color: "#9900FF",
                                binding: "rearFilter",
                            },
                        ],
                    },
                ],
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.OutputMeter,
            },
        ],
    },
});

studio.plugins.registerPluginDescription("atmoky AmbisonicRenderer", {
    companyName: "atmoky",
    productName: "atmoky AmbisonicRenderer",
    parameters: {
        headlocked: { displayName: "Headlocked" },
    },
    deckUi: {
        deckWidgetType: studio.ui.deckWidgetType.Layout,
        layout: studio.ui.layoutType.HBoxLayout,
        spacing: 10,
        items: [
            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
                minimumWidth: 160,
                spacing: 10,
                alignment: studio.ui.alignment.AlignTop,
                items: [
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Pixmap,
                        filePath: __dirname + "/atmokyTrueSpatialLogo.png",
                        alignment: studio.ui.alignment.AlignTop,
                    },
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Button,
                        binding: "headlocked",
                    },
                ],
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                layout: studio.ui.layoutType.VBoxLayout,
                minimumWidth: 128,
                maximumWidth: 250,
                spacing: 8,
                contentsMargins: { left: 4, right: 4 },
                alignment: studio.ui.alignment.AlignTop,
                isFramed: true,
                items: [
                    {
                        deckWidgetType: studio.ui.deckWidgetType.DistanceRolloffGraph,
                        rolloffTypeBinding: "DistRolloff",
                        minimumDistanceBinding: "MinDistance",
                        maximumDistanceBinding: "MaxDistance",
                        rolloffTypes: {
                            0: studio.project.distanceRolloffType.LinearSquared,
                            1: studio.project.distanceRolloffType.Linear,
                            2: studio.project.distanceRolloffType.Inverse,
                            3: studio.project.distanceRolloffType.InverseTapered,
                            4: studio.project.distanceRolloffType.Custom,
                        },
                    },
                    {
                        deckWidgetType: studio.ui.deckWidgetType.MinMaxFader,
                        text: "Min & Max Distances",
                        minimumBinding: "MinDistance",
                        maximumBinding: "MaxDistance",
                    },
                ],
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.OutputMeter,
            },
        ],
    },
});

studio.plugins.registerPluginDescription("atmoky Externalizer", {
    companyName: "atmoky",
    productName: "atmoky Externalizer",
    parameters: {
        amount: { displayName: "Amount" },
        character: { displayName: "Character" },
    },
    deckUi: {
        deckWidgetType: studio.ui.deckWidgetType.Layout,
        layout: studio.ui.layoutType.HBoxLayout,
        spacing: 10,
        items: [
            {
                deckWidgetType: studio.ui.deckWidgetType.Layout,
                alignment: studio.ui.alignment.AlignTopLeft,
                layout: studio.ui.layoutType.VBoxLayout,
                items: [
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Pixmap,
                        filePath: __dirname + "/atmokyTrueSpatialLogo.png",
                    },
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Spacer,
                    },
                    {
                        deckWidgetType: studio.ui.deckWidgetType.Layout,
                        layout: studio.ui.layoutType.VBoxLayout,
                        alignment: studio.ui.alignment.AlignCenter,
                        items: [
                            {
                                deckWidgetType: studio.ui.deckWidgetType.Layout,
                                layout: studio.ui.layoutType.HBoxLayout,
                                spacing: 20,
                                alignment: studio.ui.alignment.AlignCenter,
                                items: [
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Dial,
                                        color: "#9900FF",
                                        binding: "amount",
                                    },
                                    {
                                        deckWidgetType: studio.ui.deckWidgetType.Dial,
                                        color: "#9900FF",
                                        binding: "character",
                                    },
                                ],
                            },
                        ],
                    },
                ],
            },
            {
                deckWidgetType: studio.ui.deckWidgetType.OutputMeter,
            },
        ],
    },
});
