#include "n9933/m9933.h"
QVector<double> m9933::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
