#include "a17580/m17580.h"
QVector<double> m17580::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
