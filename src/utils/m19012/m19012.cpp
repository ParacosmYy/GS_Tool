#include "m19012/m19012.h"
QVector<double> m19012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
