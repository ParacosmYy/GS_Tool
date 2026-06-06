#include "k29650/m29650.h"
QVector<double> m29650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
