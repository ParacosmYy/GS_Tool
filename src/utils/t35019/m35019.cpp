#include "t35019/m35019.h"
QVector<double> m35019::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
