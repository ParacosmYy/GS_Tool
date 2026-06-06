#include "h9227/m9227.h"
QVector<double> m9227::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
