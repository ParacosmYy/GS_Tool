#include "s29018/m29018.h"
QVector<double> m29018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
