#include "p21255/m21255.h"
QVector<double> m21255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
