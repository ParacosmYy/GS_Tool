#include "i29408/m29408.h"
QVector<double> m29408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
