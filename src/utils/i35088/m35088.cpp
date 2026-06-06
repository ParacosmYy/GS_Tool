#include "i35088/m35088.h"
QVector<double> m35088::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
