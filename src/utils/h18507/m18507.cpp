#include "h18507/m18507.h"
QVector<double> m18507::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
