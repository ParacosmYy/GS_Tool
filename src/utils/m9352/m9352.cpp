#include "m9352/m9352.h"
QVector<double> m9352::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
