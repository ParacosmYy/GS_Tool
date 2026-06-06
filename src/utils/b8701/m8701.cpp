#include "b8701/m8701.h"
QVector<double> m8701::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
