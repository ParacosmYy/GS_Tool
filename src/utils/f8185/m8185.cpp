#include "f8185/m8185.h"
QVector<double> m8185::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
