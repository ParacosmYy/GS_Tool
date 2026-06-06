#include "f28325/m28325.h"
QVector<double> m28325::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
