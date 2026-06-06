#include "f28785/m28785.h"
QVector<double> m28785::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
