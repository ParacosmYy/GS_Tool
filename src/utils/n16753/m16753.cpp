#include "n16753/m16753.h"
QVector<double> m16753::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
