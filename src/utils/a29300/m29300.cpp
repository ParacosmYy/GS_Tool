#include "a29300/m29300.h"
QVector<double> m29300::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
