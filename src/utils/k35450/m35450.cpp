#include "k35450/m35450.h"
QVector<double> m35450::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
