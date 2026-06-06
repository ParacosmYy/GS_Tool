#include "q35016/m35016.h"
QVector<double> m35016::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
