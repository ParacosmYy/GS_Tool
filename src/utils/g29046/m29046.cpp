#include "g29046/m29046.h"
QVector<double> m29046::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
