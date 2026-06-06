#include "j8149/m8149.h"
QVector<double> m8149::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
