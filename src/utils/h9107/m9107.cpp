#include "h9107/m9107.h"
QVector<double> m9107::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
