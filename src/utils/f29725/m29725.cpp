#include "f29725/m29725.h"
QVector<double> m29725::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
