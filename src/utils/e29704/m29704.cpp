#include "e29704/m29704.h"
QVector<double> m29704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
