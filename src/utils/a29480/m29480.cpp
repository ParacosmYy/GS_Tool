#include "a29480/m29480.h"
QVector<double> m29480::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
