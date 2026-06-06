#include "f28885/m28885.h"
QVector<double> m28885::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
