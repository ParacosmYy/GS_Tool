#include "m30232/m30232.h"
QVector<double> m30232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
