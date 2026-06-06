#include "k28450/m28450.h"
QVector<double> m28450::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
