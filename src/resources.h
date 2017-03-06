// resources could be collected here

// 1v/oct for 5v = 2^5 = x32  generated by: log_gen.py
// xx=numpy.logspace(0, 5, num=1024, endpoint=True, base=2.0) # num is now many we want

static const float logspeed[1024] __attribute__ ((section (".flash"))) = {0.125000f, 0.125424f, 0.125850f, 0.126277f, 0.126705f, 0.127135f, 0.127567f, 0.128000f, 0.128434f, 0.128870f, 0.129307f, 0.129746f, 0.130186f, 0.130628f, 0.131072f, 0.131516f, 0.131963f, 0.132410f, 0.132860f, 0.133311f, 0.133763f, 0.134217f, 0.134672f, 0.135129f, 0.135588f, 0.136048f, 0.136510f, 0.136973f, 0.137438f, 0.137904f, 0.138372f, 0.138842f, 0.139313f, 0.139786f, 0.140260f, 0.140736f, 0.141214f, 0.141693f, 0.142174f, 0.142656f, 0.143140f, 0.143626f, 0.144114f, 0.144603f, 0.145093f, 0.145586f, 0.146080f, 0.146576f, 0.147073f, 0.147572f, 0.148073f, 0.148575f, 0.149080f, 0.149585f, 0.150093f, 0.150602f, 0.151114f, 0.151626f, 0.152141f, 0.152657f, 0.153175f, 0.153695f, 0.154217f, 0.154740f, 0.155265f, 0.155792f, 0.156321f, 0.156851f, 0.157383f, 0.157918f, 0.158453f, 0.158991f, 0.159531f, 0.160072f, 0.160615f, 0.161160f, 0.161707f, 0.162256f, 0.162807f, 0.163359f, 0.163914f, 0.164470f, 0.165028f, 0.165588f, 0.166150f, 0.166714f, 0.167279f, 0.167847f, 0.168417f, 0.168988f, 0.169562f, 0.170137f, 0.170715f, 0.171294f, 0.171875f, 0.172458f, 0.173044f, 0.173631f, 0.174220f, 0.174811f, 0.175405f, 0.176000f, 0.176597f, 0.177196f, 0.177798f, 0.178401f, 0.179007f, 0.179614f, 0.180224f, 0.180835f, 0.181449f, 0.182065f, 0.182682f, 0.183302f, 0.183924f, 0.184549f, 0.185175f, 0.185803f, 0.186434f, 0.187066f, 0.187701f, 0.188338f, 0.188977f, 0.189619f, 0.190262f, 0.190908f, 0.191556f, 0.192206f, 0.192858f, 0.193512f, 0.194169f, 0.194828f, 0.195489f, 0.196153f, 0.196818f, 0.197486f, 0.198156f, 0.198829f, 0.199504f, 0.200181f, 0.200860f, 0.201542f, 0.202226f, 0.202912f, 0.203600f, 0.204291f, 0.204985f, 0.205680f, 0.206378f, 0.207079f, 0.207781f, 0.208486f, 0.209194f, 0.209904f, 0.210616f, 0.211331f, 0.212048f, 0.212768f, 0.213490f, 0.214214f, 0.214941f, 0.215671f, 0.216402f, 0.217137f, 0.217874f, 0.218613f, 0.219355f, 0.220099f, 0.220846f, 0.221596f, 0.222348f, 0.223102f, 0.223859f, 0.224619f, 0.225381f, 0.226146f, 0.226914f, 0.227684f, 0.228456f, 0.229232f, 0.230010f, 0.230790f, 0.231573f, 0.232359f, 0.233148f, 0.233939f, 0.234733f, 0.235529f, 0.236329f, 0.237131f, 0.237935f, 0.238743f, 0.239553f, 0.240366f, 0.241182f, 0.242000f, 0.242821f, 0.243645f, 0.244472f, 0.245302f, 0.246134f, 0.246969f, 0.247808f, 0.248649f, 0.249492f, 0.250339f, 0.251189f, 0.252041f, 0.252896f, 0.253755f, 0.254616f, 0.255480f, 0.256347f, 0.257217f, 0.258089f, 0.258965f, 0.259844f, 0.260726f, 0.261611f, 0.262499f, 0.263389f, 0.264283f, 0.265180f, 0.266080f, 0.266983f, 0.267889f, 0.268798f, 0.269710f, 0.270625f, 0.271544f, 0.272465f, 0.273390f, 0.274318f, 0.275249f, 0.276183f, 0.277120f, 0.278060f, 0.279004f, 0.279951f, 0.280901f, 0.281854f, 0.282811f, 0.283770f, 0.284733f, 0.285700f, 0.286669f, 0.287642f, 0.288618f, 0.289598f, 0.290580f, 0.291566f, 0.292556f, 0.293549f, 0.294545f, 0.295544f, 0.296547f, 0.297554f, 0.298563f, 0.299577f, 0.300593f, 0.301613f, 0.302637f, 0.303664f, 0.304694f, 0.305728f, 0.306766f, 0.307807f, 0.308852f, 0.309900f, 0.310951f, 0.312007f, 0.313065f, 0.314128f, 0.315194f, 0.316263f, 0.317337f, 0.318414f, 0.319494f, 0.320578f, 0.321666f, 0.322758f, 0.323853f, 0.324952f, 0.326055f, 0.327161f, 0.328272f, 0.329386f, 0.330503f, 0.331625f, 0.332750f, 0.333880f, 0.335013f, 0.336150f, 0.337290f, 0.338435f, 0.339583f, 0.340736f, 0.341892f, 0.343052f, 0.344216f, 0.345385f, 0.346557f, 0.347733f, 0.348913f, 0.350097f, 0.351285f, 0.352477f, 0.353673f, 0.354873f, 0.356078f, 0.357286f, 0.358499f, 0.359715f, 0.360936f, 0.362161f, 0.363390f, 0.364623f, 0.365860f, 0.367102f, 0.368348f, 0.369598f, 0.370852f, 0.372110f, 0.373373f, 0.374640f, 0.375912f, 0.377187f, 0.378467f, 0.379752f, 0.381040f, 0.382333f, 0.383631f, 0.384933f, 0.386239f, 0.387550f, 0.388865f, 0.390185f, 0.391509f, 0.392837f, 0.394170f, 0.395508f, 0.396850f, 0.398197f, 0.399548f, 0.400904f, 0.402265f, 0.403630f, 0.405000f, 0.406374f, 0.407753f, 0.409137f, 0.410525f, 0.411918f, 0.413316f, 0.414719f, 0.416126f, 0.417538f, 0.418955f, 0.420377f, 0.421804f, 0.423235f, 0.424671f, 0.426112f, 0.427558f, 0.429009f, 0.430465f, 0.431926f, 0.433392f, 0.434863f, 0.436338f, 0.437819f, 0.439305f, 0.440796f, 0.442291f, 0.443792f, 0.445298f, 0.446810f, 0.448326f, 0.449847f, 0.451374f, 0.452906f, 0.454443f, 0.455985f, 0.457532f, 0.459085f, 0.460643f, 0.462206f, 0.463775f, 0.465348f, 0.466928f, 0.468512f, 0.470102f, 0.471697f, 0.473298f, 0.474904f, 0.476516f, 0.478133f, 0.479756f, 0.481384f, 0.483017f, 0.484656f, 0.486301f, 0.487951f, 0.489607f, 0.491269f, 0.492936f, 0.494609f, 0.496287f, 0.497971f, 0.499661f, 0.501357f, 0.503058f, 0.504766f, 0.506478f, 0.508197f, 0.509922f, 0.511652f, 0.513389f, 0.515131f, 0.516879f, 0.518633f, 0.520393f, 0.522159f, 0.523931f, 0.525709f, 0.527493f, 0.529283f, 0.531079f, 0.532881f, 0.534690f, 0.536504f, 0.538325f, 0.540152f, 0.541985f, 0.543824f, 0.545670f, 0.547521f, 0.549379f, 0.551244f, 0.553114f, 0.554992f, 0.556875f, 0.558765f, 0.560661f, 0.562564f, 0.564473f, 0.566388f, 0.568310f, 0.570239f, 0.572174f, 0.574116f, 0.576064f, 0.578019f, 0.579980f, 0.581949f, 0.583924f, 0.585905f, 0.587893f, 0.589888f, 0.591890f, 0.593899f, 0.595914f, 0.597937f, 0.599966f, 0.602002f, 0.604045f, 0.606095f, 0.608151f, 0.610215f, 0.612286f, 0.614364f, 0.616449f, 0.618541f, 0.620640f, 0.622746f, 0.624859f, 0.626980f, 0.629107f, 0.631242f, 0.633385f, 0.635534f, 0.637691f, 0.639855f, 0.642026f, 0.644205f, 0.646391f, 0.648585f, 0.650786f, 0.652994f, 0.655210f, 0.657434f, 0.659665f, 0.661903f, 0.664149f, 0.666403f, 0.668665f, 0.670934f, 0.673211f, 0.675495f, 0.677788f, 0.680088f, 0.682396f, 0.684711f, 0.687035f, 0.689367f, 0.691706f, 0.694053f, 0.696409f, 0.698772f, 0.701143f, 0.703523f, 0.705910f, 0.708306f, 0.710709f, 0.713121f, 0.715541f, 0.717969f, 0.720406f, 0.722851f, 0.725304f, 0.727765f, 0.730235f, 0.732713f, 0.735199f, 0.737694f, 0.740198f, 0.742710f, 0.745230f, 0.747759f, 0.750297f, 0.752843f, 0.755398f, 0.757961f, 0.760533f, 0.763114f, 0.765704f, 0.768302f, 0.770910f, 0.773526f, 0.776151f, 0.778785f, 0.781427f, 0.784079f, 0.786740f, 0.789410f, 0.792089f, 0.794777f, 0.797474f, 0.800180f, 0.802896f, 0.805620f, 0.808354f, 0.811097f, 0.813850f, 0.816612f, 0.819383f, 0.822164f, 0.824954f, 0.827753f, 0.830562f, 0.833381f, 0.836209f, 0.839047f, 0.841894f, 0.844751f, 0.847618f, 0.850494f, 0.853380f, 0.856276f, 0.859182f, 0.862098f, 0.865024f, 0.867959f, 0.870905f, 0.873860f, 0.876826f, 0.879801f, 0.882787f, 0.885783f, 0.888788f, 0.891805f, 0.894831f, 0.897868f, 0.900915f, 0.903972f, 0.907040f, 0.910118f, 0.913206f, 0.916305f, 0.919415f, 0.922535f, 0.925666f, 0.928807f, 0.931959f, 0.935122f, 0.938295f, 0.941479f, 0.944674f, 0.947880f, 0.951097f, 0.954324f, 0.957563f, 0.960812f, 0.964073f, 0.967344f, 0.970627f, 0.973921f, 0.977226f, 0.980542f, 0.983870f, 0.987209f, 0.990559f, 0.993920f, 0.997293f, 1.000678f, 1.004074f, 1.007481f, 1.010900f, 1.014331f, 1.017773f, 1.021227f, 1.024692f, 1.028170f, 1.031659f, 1.035160f, 1.038673f, 1.042197f, 1.045734f, 1.049283f, 1.052844f, 1.056417f, 1.060002f, 1.063599f, 1.067208f, 1.070830f, 1.074464f, 1.078110f, 1.081769f, 1.085440f, 1.089123f, 1.092819f, 1.096528f, 1.100249f, 1.103983f, 1.107729f, 1.111488f, 1.115260f, 1.119045f, 1.122842f, 1.126653f, 1.130476f, 1.134313f, 1.138162f, 1.142024f, 1.145900f, 1.149789f, 1.153690f, 1.157606f, 1.161534f, 1.165476f, 1.169431f, 1.173399f, 1.177381f, 1.181377f, 1.185386f, 1.189409f, 1.193445f, 1.197495f, 1.201559f, 1.205636f, 1.209728f, 1.213833f, 1.217952f, 1.222085f, 1.226233f, 1.230394f, 1.234569f, 1.238759f, 1.242963f, 1.247181f, 1.251413f, 1.255660f, 1.259921f, 1.264197f, 1.268487f, 1.272791f, 1.277111f, 1.281445f, 1.285793f, 1.290157f, 1.294535f, 1.298928f, 1.303336f, 1.307759f, 1.312197f, 1.316650f, 1.321118f, 1.325601f, 1.330100f, 1.334614f, 1.339143f, 1.343687f, 1.348247f, 1.352823f, 1.357413f, 1.362020f, 1.366642f, 1.371280f, 1.375933f, 1.380603f, 1.385288f, 1.389989f, 1.394706f, 1.399439f, 1.404188f, 1.408953f, 1.413735f, 1.418532f, 1.423346f, 1.428176f, 1.433023f, 1.437886f, 1.442765f, 1.447662f, 1.452574f, 1.457504f, 1.462450f, 1.467413f, 1.472392f, 1.477389f, 1.482403f, 1.487433f, 1.492481f, 1.497546f, 1.502628f, 1.507727f, 1.512844f, 1.517978f, 1.523129f, 1.528298f, 1.533484f, 1.538688f, 1.543910f, 1.549149f, 1.554406f, 1.559681f, 1.564974f, 1.570285f, 1.575614f, 1.580961f, 1.586326f, 1.591709f, 1.597111f, 1.602531f, 1.607969f, 1.613426f, 1.618901f, 1.624395f, 1.629907f, 1.635438f, 1.640988f, 1.646557f, 1.652145f, 1.657752f, 1.663377f, 1.669022f, 1.674686f, 1.680369f, 1.686071f, 1.691793f, 1.697534f, 1.703295f, 1.709075f, 1.714875f, 1.720695f, 1.726534f, 1.732393f, 1.738272f, 1.744171f, 1.750090f, 1.756029f, 1.761988f, 1.767968f, 1.773967f, 1.779987f, 1.786028f, 1.792089f, 1.798170f, 1.804273f, 1.810396f, 1.816539f, 1.822704f, 1.828889f, 1.835096f, 1.841323f, 1.847572f, 1.853842f, 1.860133f, 1.866445f, 1.872779f, 1.879135f, 1.885512f, 1.891910f, 1.898330f, 1.904773f, 1.911237f, 1.917722f, 1.924230f, 1.930760f, 1.937313f, 1.943887f, 1.950484f, 1.957103f, 1.963744f, 1.970408f, 1.977095f, 1.983804f, 1.990537f, 1.997292f, 2.004070f, 2.010870f, 2.017694f, 2.024542f, 2.031412f, 2.038306f, 2.045223f, 2.052163f, 2.059128f, 2.066115f, 2.073127f, 2.080162f, 2.087221f, 2.094304f, 2.101412f, 2.108543f, 2.115698f, 2.122878f, 2.130082f, 2.137311f, 2.144564f, 2.151842f, 2.159144f, 2.166471f, 2.173823f, 2.181200f, 2.188602f, 2.196029f, 2.203482f, 2.210959f, 2.218462f, 2.225991f, 2.233545f, 2.241125f, 2.248730f, 2.256361f, 2.264018f, 2.271701f, 2.279411f, 2.287146f, 2.294907f, 2.302695f, 2.310510f, 2.318351f, 2.326218f, 2.334112f, 2.342033f, 2.349981f, 2.357956f, 2.365958f, 2.373987f, 2.382043f, 2.390127f, 2.398238f, 2.406376f, 2.414542f, 2.422736f, 2.430958f, 2.439208f, 2.447485f, 2.455791f, 2.464125f, 2.472487f, 2.480877f, 2.489296f, 2.497744f, 2.506220f, 2.514725f, 2.523259f, 2.531822f, 2.540414f, 2.549035f, 2.557685f, 2.566365f, 2.575074f, 2.583813f, 2.592581f, 2.601379f, 2.610207f, 2.619065f, 2.627953f, 2.636871f, 2.645819f, 2.654798f, 2.663807f, 2.672847f, 2.681918f, 2.691019f, 2.700151f, 2.709314f, 2.718508f, 2.727734f, 2.736990f, 2.746279f, 2.755598f, 2.764950f, 2.774333f, 2.783747f, 2.793194f, 2.802673f, 2.812184f, 2.821728f, 2.831303f, 2.840911f, 2.850552f, 2.860226f, 2.869932f, 2.879671f, 2.889444f, 2.899249f, 2.909088f, 2.918960f, 2.928866f, 2.938805f, 2.948778f, 2.958785f, 2.968826f, 2.978901f, 2.989010f, 2.999153f, 3.009331f, 3.019543f, 3.029790f, 3.040072f, 3.050389f, 3.060741f, 3.071127f, 3.081549f, 3.092007f, 3.102500f, 3.113028f, 3.123593f, 3.134193f, 3.144829f, 3.155501f, 3.166209f, 3.176954f, 3.187735f, 3.198553f, 3.209407f, 3.220299f, 3.231227f, 3.242192f, 3.253195f, 3.264235f, 3.275312f, 3.286427f, 3.297580f, 3.308770f, 3.319999f, 3.331266f, 3.342570f, 3.353914f, 3.365295f, 3.376716f, 3.388175f, 3.399673f, 3.411210f, 3.422786f, 3.434401f, 3.446056f, 3.457751f, 3.469485f, 3.481259f, 3.493072f, 3.504926f, 3.516821f, 3.528755f, 3.540730f, 3.552746f, 3.564802f, 3.576900f, 3.589038f, 3.601218f, 3.613439f, 3.625701f, 3.638005f, 3.650351f, 3.662739f, 3.675168f, 3.687640f, 3.700155f, 3.712711f, 3.725311f, 3.737953f, 3.750638f, 3.763366f, 3.776137f, 3.788951f, 3.801809f, 3.814711f, 3.827657f, 3.840646f, 3.853679f, 3.866757f, 3.879879f, 3.893046f, 3.906257f, 3.919513f, 3.932814f, 3.946161f, 3.959552f, 3.972989f, 3.986472f, 4.000000f};

// 1v/oct for 5v = 2^5 = x32  generated by: log_gen.py
// xx=numpy.logspace(0, 5, num=1024, endpoint=True, base=2.0) # num is now many we want

static const float logpitch[128] __attribute__ ((section (".flash"))) = {0.125000f, 0.128458f, 0.132012f, 0.135664f, 0.139417f, 0.143274f, 0.147238f, 0.151311f, 0.155497f, 0.159799f, 0.164220f, 0.168763f, 0.173432f, 0.178230f, 0.183161f, 0.188228f, 0.193435f, 0.198786f, 0.204286f, 0.209937f, 0.215745f, 0.221714f, 0.227848f, 0.234151f, 0.240629f, 0.247286f, 0.254127f, 0.261158f, 0.268382f, 0.275807f, 0.283437f, 0.291279f, 0.299337f, 0.307618f, 0.316128f, 0.324874f, 0.333862f, 0.343098f, 0.352590f, 0.362344f, 0.372369f, 0.382670f, 0.393257f, 0.404136f, 0.415317f, 0.426806f, 0.438614f, 0.450748f, 0.463218f, 0.476033f, 0.489203f, 0.502736f, 0.516645f, 0.530938f, 0.545626f, 0.560721f, 0.576233f, 0.592175f, 0.608557f, 0.625393f, 0.642694f, 0.660475f, 0.678747f, 0.697524f, 0.716821f, 0.736652f, 0.757031f, 0.777975f, 0.799497f, 0.821616f, 0.844346f, 0.867704f, 0.891709f, 0.916379f, 0.941730f, 0.967783f, 0.994557f, 1.022071f, 1.050347f, 1.079405f, 1.109267f, 1.139955f, 1.171491f, 1.203901f, 1.237207f, 1.271434f, 1.306608f, 1.342756f, 1.379903f, 1.418078f, 1.457309f, 1.497626f, 1.539058f, 1.581636f, 1.625392f, 1.670358f, 1.716569f, 1.764058f, 1.812860f, 1.863013f, 1.914553f, 1.967520f, 2.021951f, 2.077888f, 2.135373f, 2.194448f, 2.255158f, 2.317547f, 2.381662f, 2.447550f, 2.515262f, 2.584847f, 2.656357f, 2.729845f, 2.805366f, 2.882976f, 2.962734f, 3.044698f, 3.128930f, 3.215492f, 3.304448f, 3.395866f, 3.489813f, 3.586358f, 3.685575f, 3.787537f, 3.892319f, 4.000000f};

// for TTS

static const unsigned char mapytoascii[]  __attribute__ ((section (".flash"))) ={32, 32, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122}; // total 64 and starts with 2 spaces SELY=0-63


/*
static const float freq[5][5] __attribute__ ((section (".flash"))) = {
      {600, 1040, 2250, 2450, 2750},
      {400, 1620, 2400, 2800, 3100},
      {250, 1750, 2600, 3050, 3340},
      {400, 750, 2400, 2600, 2900},
      {350, 600, 2400, 2675, 2950}
  };

static const float qqq[5][5] __attribute__ ((section (".flash"))) = {
    {14.424072,21.432398,29.508234,29.453644,30.517193},
    {14.424072,29.213112,34.623474,33.661621,37.268467},
    {6.004305,28.050945,37.508934,36.667324,40.153900},
    {14.424072,13.522194,34.623474,31.257082,34.863983},
{12.620288,10.816360,34.623474,32.158768,35.465038}
  };

static const float mull[5][5] __attribute__ ((section (".flash"))) = {
    {1, 0.44668359215096, 0.35481338923358, 0.35481338923358, 0.1},
    {1, 0.25118864315096, 0.35481338923358, 0.25118864315096, 0.12589254117942},
    {1, 0.031622776601684, 0.15848931924611, 0.079432823472428, 0.03981071705535},
    {1, 0.28183829312645, 0.089125093813375, 0.1, 0.01},
    { 1, 0.1, 0.025118864315096, 0.03981071705535, 0.015848931924611}
  };

*/