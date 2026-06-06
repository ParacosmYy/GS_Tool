#include "a25640/m25640.h"
QVector<double> m25640::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
