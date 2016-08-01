const float sintables[] __attribute__ ((section (".flash"))) = {0.000000, 0.003068, 0.006136, 0.009204, 0.012272, 0.015339, 0.018407, 0.021474, 0.024541, 0.027608, 0.030675, 0.033741, 0.036807, 0.039873, 0.042938, 0.046003, 0.049068, 0.052132, 0.055195, 0.058258, 0.061321, 0.064383, 0.067444, 0.070505, 0.073565, 0.076624, 0.079682, 0.082740, 0.085797, 0.088854, 0.091909, 0.094963, 0.098017, 0.101070, 0.104122, 0.107172, 0.110222, 0.113271, 0.116319, 0.119365, 0.122411, 0.125455, 0.128498, 0.131540, 0.134581, 0.137620, 0.140658, 0.143695, 0.146730, 0.149765, 0.152797, 0.155828, 0.158858, 0.161886, 0.164913, 0.167938, 0.170962, 0.173984, 0.177004, 0.180023, 0.183040, 0.186055, 0.189069, 0.192080, 0.195090, 0.198098, 0.201105, 0.204109, 0.207111, 0.210112, 0.213110, 0.216107, 0.219101, 0.222094, 0.225084, 0.228072, 0.231058, 0.234042, 0.237024, 0.240003, 0.242980, 0.245955, 0.248928, 0.251898, 0.254866, 0.257831, 0.260794, 0.263755, 0.266713, 0.269668, 0.272621, 0.275572, 0.278520, 0.281465, 0.284408, 0.287347, 0.290285, 0.293219, 0.296151, 0.299080, 0.302006, 0.304929, 0.307850, 0.310767, 0.313682, 0.316593, 0.319502, 0.322408, 0.325310, 0.328210, 0.331106, 0.334000, 0.336890, 0.339777, 0.342661, 0.345541, 0.348419, 0.351293, 0.354164, 0.357031, 0.359895, 0.362756, 0.365613, 0.368467, 0.371317, 0.374164, 0.377007, 0.379847, 0.382683, 0.385516, 0.388345, 0.391170, 0.393992, 0.396810, 0.399624, 0.402435, 0.405241, 0.408044, 0.410843, 0.413638, 0.416430, 0.419217, 0.422000, 0.424780, 0.427555, 0.430326, 0.433094, 0.435857, 0.438616, 0.441371, 0.444122, 0.446869, 0.449611, 0.452350, 0.455084, 0.457813, 0.460539, 0.463260, 0.465977, 0.468689, 0.471397, 0.474100, 0.476799, 0.479494, 0.482184, 0.484869, 0.487550, 0.490227, 0.492898, 0.495565, 0.498228, 0.500885, 0.503538, 0.506187, 0.508830, 0.511469, 0.514103, 0.516732, 0.519356, 0.521975, 0.524590, 0.527199, 0.529804, 0.532403, 0.534998, 0.537587, 0.540172, 0.542751, 0.545325, 0.547894, 0.550458, 0.553017, 0.555570, 0.558119, 0.560662, 0.563199, 0.565732, 0.568259, 0.570781, 0.573297, 0.575808, 0.578314, 0.580814, 0.583309, 0.585798, 0.588282, 0.590760, 0.593232, 0.595699, 0.598161, 0.600616, 0.603067, 0.605511, 0.607950, 0.610383, 0.612810, 0.615232, 0.617647, 0.620057, 0.622461, 0.624860, 0.627252, 0.629638, 0.632019, 0.634393, 0.636762, 0.639124, 0.641481, 0.643832, 0.646176, 0.648514, 0.650847, 0.653173, 0.655493, 0.657807, 0.660114, 0.662416, 0.664711, 0.667000, 0.669283, 0.671559, 0.673829, 0.676093, 0.678350, 0.680601, 0.682846, 0.685084, 0.687315, 0.689541, 0.691759, 0.693971, 0.696177, 0.698376, 0.700569, 0.702755, 0.704934, 0.707107, 0.709273, 0.711432, 0.713585, 0.715731, 0.717870, 0.720003, 0.722128, 0.724247, 0.726359, 0.728464, 0.730563, 0.732654, 0.734739, 0.736817, 0.738887, 0.740951, 0.743008, 0.745058, 0.747101, 0.749136, 0.751165, 0.753187, 0.755201, 0.757209, 0.759209, 0.761202, 0.763188, 0.765167, 0.767139, 0.769103, 0.771061, 0.773010, 0.774953, 0.776888, 0.778817, 0.780737, 0.782651, 0.784557, 0.786455, 0.788346, 0.790230, 0.792107, 0.793975, 0.795837, 0.797691, 0.799537, 0.801376, 0.803208, 0.805031, 0.806848, 0.808656, 0.810457, 0.812251, 0.814036, 0.815814, 0.817585, 0.819348, 0.821102, 0.822850, 0.824589, 0.826321, 0.828045, 0.829761, 0.831470, 0.833170, 0.834863, 0.836548, 0.838225, 0.839894, 0.841555, 0.843208, 0.844854, 0.846491, 0.848120, 0.849742, 0.851355, 0.852961, 0.854558, 0.856147, 0.857729, 0.859302, 0.860867, 0.862424, 0.863973, 0.865514, 0.867046, 0.868571, 0.870087, 0.871595, 0.873095, 0.874587, 0.876070, 0.877545, 0.879012, 0.880471, 0.881921, 0.883363, 0.884797, 0.886223, 0.887640, 0.889048, 0.890449, 0.891841, 0.893224, 0.894599, 0.895966, 0.897325, 0.898674, 0.900016, 0.901349, 0.902673, 0.903989, 0.905297, 0.906596, 0.907886, 0.909168, 0.910441, 0.911706, 0.912962, 0.914210, 0.915449, 0.916679, 0.917901, 0.919114, 0.920318, 0.921514, 0.922701, 0.923880, 0.925049, 0.926210, 0.927363, 0.928506, 0.929641, 0.930767, 0.931884, 0.932993, 0.934093, 0.935184, 0.936266, 0.937339, 0.938404, 0.939459, 0.940506, 0.941544, 0.942573, 0.943593, 0.944605, 0.945607, 0.946601, 0.947586, 0.948561, 0.949528, 0.950486, 0.951435, 0.952375, 0.953306, 0.954228, 0.955141, 0.956045, 0.956940, 0.957826, 0.958703, 0.959571, 0.960431, 0.961280, 0.962121, 0.962953, 0.963776, 0.964590, 0.965394, 0.966190, 0.966976, 0.967754, 0.968522, 0.969281, 0.970031, 0.970772, 0.971504, 0.972227, 0.972940, 0.973644, 0.974339, 0.975025, 0.975702, 0.976370, 0.977028, 0.977677, 0.978317, 0.978948, 0.979570, 0.980182, 0.980785, 0.981379, 0.981964, 0.982539, 0.983105, 0.983662, 0.984210, 0.984748, 0.985278, 0.985798, 0.986308, 0.986809, 0.987301, 0.987784, 0.988258, 0.988722, 0.989177, 0.989622, 0.990058, 0.990485, 0.990903, 0.991311, 0.991710, 0.992099, 0.992480, 0.992850, 0.993212, 0.993564, 0.993907, 0.994240, 0.994565, 0.994879, 0.995185, 0.995481, 0.995767, 0.996045, 0.996313, 0.996571, 0.996820, 0.997060, 0.997290, 0.997511, 0.997723, 0.997925, 0.998118, 0.998302, 0.998476, 0.998640, 0.998795, 0.998941, 0.999078, 0.999205, 0.999322, 0.999431, 0.999529, 0.999619, 0.999699, 0.999769, 0.999831, 0.999882, 0.999925, 0.999958, 0.999981, 0.999995, 1.000000, 0.999995, 0.999981, 0.999958, 0.999925, 0.999882, 0.999831, 0.999769, 0.999699, 0.999619, 0.999529, 0.999431, 0.999322, 0.999205, 0.999078, 0.998941, 0.998795, 0.998640, 0.998476, 0.998302, 0.998118, 0.997925, 0.997723, 0.997511, 0.997290, 0.997060, 0.996820, 0.996571, 0.996313, 0.996045, 0.995767, 0.995481, 0.995185, 0.994879, 0.994565, 0.994240, 0.993907, 0.993564, 0.993212, 0.992850, 0.992480, 0.992099, 0.991710, 0.991311, 0.990903, 0.990485, 0.990058, 0.989622, 0.989177, 0.988722, 0.988258, 0.987784, 0.987301, 0.986809, 0.986308, 0.985798, 0.985278, 0.984748, 0.984210, 0.983662, 0.983105, 0.982539, 0.981964, 0.981379, 0.980785, 0.980182, 0.979570, 0.978948, 0.978317, 0.977677, 0.977028, 0.976370, 0.975702, 0.975025, 0.974339, 0.973644, 0.972940, 0.972227, 0.971504, 0.970772, 0.970031, 0.969281, 0.968522, 0.967754, 0.966976, 0.966190, 0.965394, 0.964590, 0.963776, 0.962953, 0.962121, 0.961280, 0.960431, 0.959571, 0.958703, 0.957826, 0.956940, 0.956045, 0.955141, 0.954228, 0.953306, 0.952375, 0.951435, 0.950486, 0.949528, 0.948561, 0.947586, 0.946601, 0.945607, 0.944605, 0.943593, 0.942573, 0.941544, 0.940506, 0.939459, 0.938404, 0.937339, 0.936266, 0.935184, 0.934093, 0.932993, 0.931884, 0.930767, 0.929641, 0.928506, 0.927363, 0.926210, 0.925049, 0.923880, 0.922701, 0.921514, 0.920318, 0.919114, 0.917901, 0.916679, 0.915449, 0.914210, 0.912962, 0.911706, 0.910441, 0.909168, 0.907886, 0.906596, 0.905297, 0.903989, 0.902673, 0.901349, 0.900016, 0.898674, 0.897325, 0.895966, 0.894599, 0.893224, 0.891841, 0.890449, 0.889048, 0.887640, 0.886222, 0.884797, 0.883363, 0.881921, 0.880471, 0.879012, 0.877545, 0.876070, 0.874587, 0.873095, 0.871595, 0.870087, 0.868571, 0.867046, 0.865514, 0.863973, 0.862424, 0.860867, 0.859302, 0.857729, 0.856147, 0.854558, 0.852961, 0.851355, 0.849742, 0.848120, 0.846491, 0.844854, 0.843208, 0.841555, 0.839894, 0.838225, 0.836548, 0.834863, 0.833170, 0.831470, 0.829761, 0.828045, 0.826321, 0.824589, 0.822850, 0.821102, 0.819348, 0.817585, 0.815814, 0.814036, 0.812251, 0.810457, 0.808656, 0.806848, 0.805031, 0.803208, 0.801376, 0.799537, 0.797691, 0.795837, 0.793975, 0.792107, 0.790230, 0.788346, 0.786455, 0.784557, 0.782651, 0.780737, 0.778817, 0.776888, 0.774953, 0.773010, 0.771061, 0.769103, 0.767139, 0.765167, 0.763188, 0.761202, 0.759209, 0.757209, 0.755201, 0.753187, 0.751165, 0.749136, 0.747101, 0.745058, 0.743008, 0.740951, 0.738887, 0.736817, 0.734739, 0.732654, 0.730563, 0.728464, 0.726359, 0.724247, 0.722128, 0.720003, 0.717870, 0.715731, 0.713585, 0.711432, 0.709273, 0.707107, 0.704934, 0.702755, 0.700569, 0.698376, 0.696177, 0.693971, 0.691759, 0.689541, 0.687315, 0.685084, 0.682845, 0.680601, 0.678350, 0.676093, 0.673829, 0.671559, 0.669283, 0.667000, 0.664711, 0.662416, 0.660114, 0.657807, 0.655493, 0.653173, 0.650847, 0.648514, 0.646176, 0.643832, 0.641481, 0.639124, 0.636762, 0.634393, 0.632019, 0.629638, 0.627252, 0.624859, 0.622461, 0.620057, 0.617647, 0.615232, 0.612810, 0.610383, 0.607950, 0.605511, 0.603067, 0.600617, 0.598161, 0.595699, 0.593232, 0.590760, 0.588282, 0.585798, 0.583309, 0.580814, 0.578314, 0.575808, 0.573297, 0.570781, 0.568259, 0.565732, 0.563199, 0.560661, 0.558118, 0.555570, 0.553017, 0.550458, 0.547894, 0.545325, 0.542751, 0.540172, 0.537587, 0.534998, 0.532403, 0.529804, 0.527199, 0.524590, 0.521975, 0.519356, 0.516732, 0.514103, 0.511469, 0.508830, 0.506187, 0.503538, 0.500885, 0.498228, 0.495565, 0.492898, 0.490226, 0.487550, 0.484869, 0.482184, 0.479494, 0.476799, 0.474100, 0.471397, 0.468689, 0.465977, 0.463260, 0.460539, 0.457813, 0.455084, 0.452350, 0.449611, 0.446869, 0.444122, 0.441371, 0.438616, 0.435857, 0.433094, 0.430326, 0.427555, 0.424780, 0.422000, 0.419217, 0.416429, 0.413638, 0.410843, 0.408044, 0.405241, 0.402435, 0.399624, 0.396810, 0.393992, 0.391170, 0.388345, 0.385516, 0.382683, 0.379847, 0.377007, 0.374164, 0.371317, 0.368467, 0.365613, 0.362756, 0.359895, 0.357031, 0.354163, 0.351293, 0.348419, 0.345541, 0.342661, 0.339777, 0.336890, 0.334000, 0.331106, 0.328210, 0.325310, 0.322408, 0.319502, 0.316593, 0.313682, 0.310767, 0.307850, 0.304929, 0.302006, 0.299080, 0.296151, 0.293219, 0.290285, 0.287347, 0.284408, 0.281465, 0.278520, 0.275572, 0.272621, 0.269668, 0.266713, 0.263755, 0.260794, 0.257831, 0.254866, 0.251898, 0.248928, 0.245955, 0.242980, 0.240003, 0.237024, 0.234042, 0.231058, 0.228072, 0.225084, 0.222094, 0.219101, 0.216107, 0.213110, 0.210112, 0.207111, 0.204109, 0.201105, 0.198098, 0.195090, 0.192080, 0.189069, 0.186055, 0.183040, 0.180023, 0.177004, 0.173984, 0.170962, 0.167938, 0.164913, 0.161886, 0.158858, 0.155829, 0.152797, 0.149765, 0.146731, 0.143695, 0.140658, 0.137620, 0.134581, 0.131540, 0.128498, 0.125455, 0.122411, 0.119365, 0.116319, 0.113271, 0.110222, 0.107172, 0.104122, 0.101070, 0.098017, 0.094963, 0.091909, 0.088853, 0.085797, 0.082740, 0.079682, 0.076624, 0.073564, 0.070505, 0.067444, 0.064383, 0.061321, 0.058258, 0.055195, 0.052132, 0.049068, 0.046003, 0.042938, 0.039873, 0.036807, 0.033741, 0.030675, 0.027608, 0.024541, 0.021474, 0.018407, 0.015339, 0.012271, 0.009204, 0.006136, 0.003068, -0.000000, -0.003068, -0.006136, -0.009204, -0.012271, -0.015339, -0.018407, -0.021474, -0.024541, -0.027608, -0.030675, -0.033741, -0.036807, -0.039873, -0.042938, -0.046003, -0.049068, -0.052132, -0.055195, -0.058258, -0.061321, -0.064383, -0.067444, -0.070505, -0.073565, -0.076624, -0.079683, -0.082740, -0.085797, -0.088854, -0.091909, -0.094964, -0.098017, -0.101070, -0.104122, -0.107172, -0.110222, -0.113271, -0.116319, -0.119365, -0.122411, -0.125455, -0.128498, -0.131540, -0.134581, -0.137620, -0.140658, -0.143695, -0.146730, -0.149765, -0.152797, -0.155828, -0.158858, -0.161886, -0.164913, -0.167938, -0.170962, -0.173984, -0.177004, -0.180023, -0.183040, -0.186055, -0.189069, -0.192080, -0.195090, -0.198098, -0.201105, -0.204109, -0.207111, -0.210112, -0.213110, -0.216107, -0.219101, -0.222094, -0.225084, -0.228072, -0.231058, -0.234042, -0.237024, -0.240003, -0.242980, -0.245955, -0.248928, -0.251898, -0.254866, -0.257831, -0.260794, -0.263755, -0.266713, -0.269668, -0.272621, -0.275572, -0.278520, -0.281465, -0.284407, -0.287347, -0.290285, -0.293219, -0.296151, -0.299080, -0.302006, -0.304929, -0.307850, -0.310767, -0.313682, -0.316593, -0.319502, -0.322408, -0.325310, -0.328210, -0.331106, -0.334000, -0.336890, -0.339777, -0.342661, -0.345541, -0.348419, -0.351293, -0.354163, -0.357031, -0.359895, -0.362756, -0.365613, -0.368467, -0.371317, -0.374164, -0.377007, -0.379847, -0.382683, -0.385516, -0.388345, -0.391170, -0.393992, -0.396810, -0.399624, -0.402435, -0.405241, -0.408044, -0.410843, -0.413638, -0.416430, -0.419217, -0.422000, -0.424780, -0.427555, -0.430326, -0.433094, -0.435857, -0.438616, -0.441371, -0.444122, -0.446869, -0.449611, -0.452350, -0.455084, -0.457813, -0.460539, -0.463260, -0.465977, -0.468689, -0.471397, -0.474100, -0.476799, -0.479494, -0.482184, -0.484869, -0.487550, -0.490227, -0.492898, -0.495565, -0.498228, -0.500885, -0.503538, -0.506187, -0.508830, -0.511469, -0.514103, -0.516732, -0.519356, -0.521975, -0.524590, -0.527199, -0.529804, -0.532403, -0.534998, -0.537587, -0.540172, -0.542751, -0.545325, -0.547894, -0.550458, -0.553017, -0.555570, -0.558119, -0.560662, -0.563199, -0.565732, -0.568259, -0.570781, -0.573297, -0.575808, -0.578314, -0.580814, -0.583309, -0.585798, -0.588282, -0.590760, -0.593232, -0.595699, -0.598161, -0.600617, -0.603067, -0.605511, -0.607950, -0.610383, -0.612810, -0.615232, -0.617647, -0.620057, -0.622461, -0.624860, -0.627252, -0.629638, -0.632019, -0.634393, -0.636762, -0.639124, -0.641481, -0.643831, -0.646176, -0.648514, -0.650847, -0.653173, -0.655493, -0.657807, -0.660114, -0.662416, -0.664711, -0.667000, -0.669283, -0.671559, -0.673829, -0.676093, -0.678350, -0.680601, -0.682846, -0.685084, -0.687315, -0.689541, -0.691759, -0.693972, -0.696177, -0.698376, -0.700569, -0.702755, -0.704934, -0.707107, -0.709273, -0.711432, -0.713585, -0.715731, -0.717870, -0.720002, -0.722128, -0.724247, -0.726359, -0.728464, -0.730563, -0.732654, -0.734739, -0.736817, -0.738887, -0.740951, -0.743008, -0.745058, -0.747101, -0.749136, -0.751165, -0.753187, -0.755201, -0.757209, -0.759209, -0.761202, -0.763188, -0.765167, -0.767139, -0.769103, -0.771061, -0.773010, -0.774953, -0.776888, -0.778817, -0.780737, -0.782651, -0.784557, -0.786455, -0.788346, -0.790230, -0.792107, -0.793976, -0.795837, -0.797691, -0.799537, -0.801376, -0.803208, -0.805031, -0.806848, -0.808656, -0.810457, -0.812251, -0.814036, -0.815814, -0.817585, -0.819348, -0.821103, -0.822850, -0.824589, -0.826321, -0.828045, -0.829761, -0.831469, -0.833170, -0.834863, -0.836548, -0.838225, -0.839894, -0.841555, -0.843208, -0.844853, -0.846491, -0.848120, -0.849742, -0.851355, -0.852961, -0.854558, -0.856147, -0.857729, -0.859302, -0.860867, -0.862424, -0.863973, -0.865514, -0.867046, -0.868571, -0.870087, -0.871595, -0.873095, -0.874587, -0.876070, -0.877545, -0.879012, -0.880471, -0.881921, -0.883363, -0.884797, -0.886223, -0.887640, -0.889048, -0.890449, -0.891841, -0.893224, -0.894600, -0.895966, -0.897325, -0.898675, -0.900016, -0.901349, -0.902673, -0.903989, -0.905297, -0.906596, -0.907886, -0.909168, -0.910441, -0.911706, -0.912962, -0.914210, -0.915449, -0.916679, -0.917901, -0.919114, -0.920318, -0.921514, -0.922701, -0.923880, -0.925049, -0.926210, -0.927363, -0.928506, -0.929641, -0.930767, -0.931884, -0.932993, -0.934093, -0.935183, -0.936266, -0.937339, -0.938404, -0.939459, -0.940506, -0.941544, -0.942573, -0.943593, -0.944605, -0.945607, -0.946601, -0.947586, -0.948561, -0.949528, -0.950486, -0.951435, -0.952375, -0.953306, -0.954228, -0.955141, -0.956045, -0.956940, -0.957826, -0.958704, -0.959572, -0.960431, -0.961281, -0.962121, -0.962953, -0.963776, -0.964590, -0.965394, -0.966190, -0.966977, -0.967754, -0.968522, -0.969281, -0.970031, -0.970772, -0.971504, -0.972226, -0.972940, -0.973644, -0.974339, -0.975025, -0.975702, -0.976370, -0.977028, -0.977677, -0.978317, -0.978948, -0.979570, -0.980182, -0.980785, -0.981379, -0.981964, -0.982539, -0.983105, -0.983662, -0.984210, -0.984748, -0.985278, -0.985798, -0.986308, -0.986809, -0.987301, -0.987784, -0.988258, -0.988722, -0.989177, -0.989622, -0.990058, -0.990485, -0.990903, -0.991311, -0.991710, -0.992099, -0.992480, -0.992850, -0.993212, -0.993564, -0.993907, -0.994240, -0.994565, -0.994879, -0.995185, -0.995481, -0.995767, -0.996045, -0.996313, -0.996571, -0.996820, -0.997060, -0.997290, -0.997511, -0.997723, -0.997925, -0.998118, -0.998302, -0.998476, -0.998640, -0.998795, -0.998941, -0.999078, -0.999205, -0.999322, -0.999431, -0.999529, -0.999619, -0.999699, -0.999769, -0.999831, -0.999882, -0.999925, -0.999958, -0.999981, -0.999995, -1.000000, -0.999995, -0.999981, -0.999958, -0.999925, -0.999882, -0.999831, -0.999769, -0.999699, -0.999619, -0.999529, -0.999431, -0.999322, -0.999205, -0.999078, -0.998941, -0.998795, -0.998640, -0.998476, -0.998302, -0.998118, -0.997925, -0.997723, -0.997511, -0.997290, -0.997060, -0.996820, -0.996571, -0.996313, -0.996045, -0.995767, -0.995481, -0.995185, -0.994879, -0.994565, -0.994240, -0.993907, -0.993564, -0.993212, -0.992850, -0.992480, -0.992099, -0.991710, -0.991311, -0.990903, -0.990485, -0.990058, -0.989622, -0.989177, -0.988722, -0.988258, -0.987784, -0.987301, -0.986809, -0.986308, -0.985798, -0.985278, -0.984748, -0.984210, -0.983662, -0.983105, -0.982539, -0.981964, -0.981379, -0.980785, -0.980182, -0.979570, -0.978948, -0.978317, -0.977677, -0.977028, -0.976370, -0.975702, -0.975025, -0.974339, -0.973644, -0.972940, -0.972226, -0.971504, -0.970772, -0.970031, -0.969281, -0.968522, -0.967754, -0.966977, -0.966190, -0.965394, -0.964590, -0.963776, -0.962953, -0.962121, -0.961281, -0.960431, -0.959572, -0.958704, -0.957826, -0.956940, -0.956045, -0.955141, -0.954228, -0.953306, -0.952375, -0.951435, -0.950486, -0.949528, -0.948561, -0.947586, -0.946601, -0.945607, -0.944605, -0.943593, -0.942573, -0.941544, -0.940506, -0.939459, -0.938403, -0.937339, -0.936266, -0.935183, -0.934093, -0.932993, -0.931884, -0.930767, -0.929641, -0.928506, -0.927362, -0.926210, -0.925049, -0.923879, -0.922701, -0.921514, -0.920318, -0.919114, -0.917901, -0.916679, -0.915449, -0.914210, -0.912962, -0.911706, -0.910441, -0.909168, -0.907886, -0.906596, -0.905297, -0.903989, -0.902673, -0.901349, -0.900016, -0.898675, -0.897325, -0.895966, -0.894600, -0.893224, -0.891841, -0.890449, -0.889048, -0.887640, -0.886223, -0.884797, -0.883363, -0.881921, -0.880471, -0.879012, -0.877545, -0.876070, -0.874587, -0.873095, -0.871595, -0.870087, -0.868571, -0.867046, -0.865514, -0.863973, -0.862424, -0.860867, -0.859302, -0.857729, -0.856147, -0.854558, -0.852961, -0.851355, -0.849742, -0.848120, -0.846491, -0.844853, -0.843208, -0.841555, -0.839894, -0.838225, -0.836548, -0.834863, -0.833170, -0.831470, -0.829761, -0.828045, -0.826321, -0.824589, -0.822850, -0.821103, -0.819348, -0.817585, -0.815814, -0.814036, -0.812251, -0.810457, -0.808656, -0.806848, -0.805031, -0.803208, -0.801376, -0.799537, -0.797691, -0.795837, -0.793976, -0.792107, -0.790230, -0.788346, -0.786455, -0.784557, -0.782651, -0.780737, -0.778817, -0.776888, -0.774953, -0.773010, -0.771060, -0.769103, -0.767139, -0.765167, -0.763188, -0.761202, -0.759209, -0.757209, -0.755201, -0.753187, -0.751165, -0.749136, -0.747100, -0.745058, -0.743008, -0.740951, -0.738887, -0.736816, -0.734739, -0.732654, -0.730563, -0.728465, -0.726359, -0.724247, -0.722128, -0.720003, -0.717870, -0.715731, -0.713585, -0.711432, -0.709273, -0.707107, -0.704934, -0.702755, -0.700569, -0.698376, -0.696177, -0.693972, -0.691759, -0.689541, -0.687315, -0.685084, -0.682846, -0.680601, -0.678350, -0.676093, -0.673829, -0.671559, -0.669283, -0.667000, -0.664711, -0.662416, -0.660114, -0.657807, -0.655493, -0.653173, -0.650847, -0.648514, -0.646176, -0.643831, -0.641481, -0.639124, -0.636762, -0.634393, -0.632019, -0.629638, -0.627252, -0.624859, -0.622461, -0.620057, -0.617647, -0.615231, -0.612810, -0.610383, -0.607950, -0.605511, -0.603067, -0.600617, -0.598161, -0.595699, -0.593232, -0.590760, -0.588282, -0.585798, -0.583309, -0.580814, -0.578314, -0.575808, -0.573297, -0.570781, -0.568259, -0.565732, -0.563199, -0.560662, -0.558119, -0.555570, -0.553017, -0.550458, -0.547894, -0.545325, -0.542751, -0.540172, -0.537587, -0.534998, -0.532403, -0.529804, -0.527199, -0.524590, -0.521975, -0.519356, -0.516732, -0.514103, -0.511469, -0.508830, -0.506187, -0.503538, -0.500885, -0.498228, -0.495565, -0.492898, -0.490226, -0.487550, -0.484869, -0.482184, -0.479494, -0.476799, -0.474100, -0.471397, -0.468689, -0.465976, -0.463260, -0.460539, -0.457814, -0.455084, -0.452350, -0.449612, -0.446869, -0.444122, -0.441371, -0.438616, -0.435857, -0.433094, -0.430327, -0.427555, -0.424780, -0.422000, -0.419217, -0.416430, -0.413638, -0.410843, -0.408044, -0.405241, -0.402435, -0.399624, -0.396810, -0.393992, -0.391170, -0.388345, -0.385516, -0.382683, -0.379847, -0.377007, -0.374164, -0.371317, -0.368467, -0.365613, -0.362756, -0.359895, -0.357031, -0.354163, -0.351293, -0.348419, -0.345541, -0.342661, -0.339777, -0.336890, -0.333999, -0.331106, -0.328210, -0.325310, -0.322407, -0.319502, -0.316593, -0.313682, -0.310767, -0.307850, -0.304929, -0.302006, -0.299080, -0.296151, -0.293219, -0.290285, -0.287348, -0.284408, -0.281465, -0.278520, -0.275572, -0.272621, -0.269668, -0.266713, -0.263755, -0.260794, -0.257831, -0.254866, -0.251898, -0.248928, -0.245955, -0.242980, -0.240003, -0.237024, -0.234042, -0.231058, -0.228072, -0.225084, -0.222094, -0.219101, -0.216107, -0.213110, -0.210112, -0.207111, -0.204109, -0.201105, -0.198098, -0.195090, -0.192080, -0.189069, -0.186055, -0.183040, -0.180023, -0.177004, -0.173984, -0.170962, -0.167938, -0.164913, -0.161886, -0.158858, -0.155828, -0.152797, -0.149764, -0.146730, -0.143695, -0.140658, -0.137620, -0.134581, -0.131540, -0.128498, -0.125455, -0.122411, -0.119365, -0.116319, -0.113271, -0.110222, -0.107173, -0.104122, -0.101070, -0.098017, -0.094964, -0.091909, -0.088854, -0.085797, -0.082740, -0.079682, -0.076624, -0.073565, -0.070505, -0.067444, -0.064383, -0.061321, -0.058258, -0.055195, -0.052132, -0.049068, -0.046003, -0.042938, -0.039873, -0.036807, -0.033741, -0.030675, -0.027608, -0.024541, -0.021474, -0.018407, -0.015339, -0.012271, -0.009204, -0.006136, -0.003068};
