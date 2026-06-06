#include "s19018/m19018.h"
QVector<double> m19018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
