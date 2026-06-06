#include "t29099/m29099.h"
QVector<double> m29099::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
