#include "j8009/m8009.h"
QVector<double> m8009::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
