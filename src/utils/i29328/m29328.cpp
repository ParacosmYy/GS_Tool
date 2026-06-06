#include "i29328/m29328.h"
QVector<double> m29328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
