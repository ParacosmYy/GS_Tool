#include "m29992/m29992.h"
QVector<double> m29992::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
