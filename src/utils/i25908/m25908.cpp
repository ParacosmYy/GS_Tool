#include "i25908/m25908.h"
QVector<double> m25908::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
