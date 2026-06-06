#include "a28920/m28920.h"
QVector<double> m28920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
