#include "e35404/m35404.h"
QVector<double> m35404::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
