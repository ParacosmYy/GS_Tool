#include "a35860/m35860.h"
QVector<double> m35860::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
