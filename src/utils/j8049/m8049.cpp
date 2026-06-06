#include "j8049/m8049.h"
QVector<double> m8049::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
