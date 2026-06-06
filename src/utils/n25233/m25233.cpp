#include "n25233/m25233.h"
QVector<double> m25233::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
