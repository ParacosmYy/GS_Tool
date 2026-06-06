#include "e8584/m8584.h"
QVector<double> m8584::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
