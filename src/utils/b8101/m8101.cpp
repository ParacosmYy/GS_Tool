#include "b8101/m8101.h"
QVector<double> m8101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
