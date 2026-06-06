#include "k35930/m35930.h"
QVector<double> m35930::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
