#include "g29006/m29006.h"
QVector<double> m29006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
