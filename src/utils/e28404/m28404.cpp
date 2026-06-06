#include "e28404/m28404.h"
QVector<double> m28404::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
