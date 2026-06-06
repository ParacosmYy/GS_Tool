#include "j8749/m8749.h"
QVector<double> m8749::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
