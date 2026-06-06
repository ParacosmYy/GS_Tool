#include "j8309/m8309.h"
QVector<double> m8309::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
