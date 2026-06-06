#include "h24107/m24107.h"
QVector<double> m24107::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
