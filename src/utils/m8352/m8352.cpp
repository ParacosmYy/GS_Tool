#include "m8352/m8352.h"
QVector<double> m8352::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
