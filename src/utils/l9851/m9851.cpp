#include "l9851/m9851.h"
QVector<double> m9851::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
