#include "i29008/m29008.h"
QVector<double> m29008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
